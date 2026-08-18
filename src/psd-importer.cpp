#include "psd-importer.hpp"
#include "psd-bridge.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUuid>

#include <algorithm>
#include <cstdint>

namespace wg {
namespace {

QString makeId()
{
  return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString safeFileName(QString value)
{
  value = value.trimmed();
  value.replace(QRegularExpression(R"([^A-Za-z0-9_\-]+)"), "_");
  if (value.isEmpty())
    value = "layer";
  return value.left(80);
}

QString layerName(wg_psd_doc *doc, int32_t index)
{
  QByteArray buffer(4096, '\0');
  if (wg_psd_layer_name(doc, index, buffer.data(),
                        static_cast<size_t>(buffer.size())) != 0 ||
      buffer.constData()[0] == '\0') {
    return QString("Capa %1").arg(index + 1);
  }

  return QString::fromUtf8(buffer.constData());
}

TextHorizontalAlign horizontalAlignment(int value)
{
  switch (value) {
  case 1:
    return TextHorizontalAlign::Right;
  case 2:
    return TextHorizontalAlign::Center;
  default:
    return TextHorizontalAlign::Left;
  }
}

QString importedAssetDirectory(const QString &sourceFile)
{
  const QString base =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
      "/worship-graphics/imported";

  const QString folder =
      safeFileName(QFileInfo(sourceFile).completeBaseName()) + "_" +
      QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);

  const QString result = base + "/" + folder;
  QDir().mkpath(result);
  return result;
}

Layer commonLayer(wg_psd_doc *doc,
                  int32_t index,
                  const QString &name,
                  const QString &parentId)
{
  int32_t top = 0;
  int32_t left = 0;
  int32_t bottom = 0;
  int32_t right = 0;
  uint8_t opacity = 255;
  uint8_t flags = 0;

  wg_psd_layer_bounds(doc, index, &top, &left, &bottom, &right);
  wg_psd_layer_properties(doc, index, &opacity, &flags);

  Layer layer;
  layer.id = makeId();
  layer.name = name;
  layer.parentId = parentId;
  layer.position = QPointF(left, top);
  layer.size = QSizeF(std::max(1, right - left),
                      std::max(1, bottom - top));
  layer.opacity = opacity / 255.0;
  layer.visible = (flags & 0x02u) == 0;
  layer.locked = false;
  layer.enterAnimation = AnimationPreset::None;
  layer.exitAnimation = AnimationPreset::None;
  layer.enterDelayMs = 0;
  layer.exitDelayMs = 0;
  layer.enterDurationMs = 300;
  layer.exitDurationMs = 300;
  return layer;
}

bool saveRasterLayer(wg_psd_doc *doc,
                     int32_t index,
                     const QString &assetDir,
                     const QString &name,
                     int width,
                     int height,
                     QString *outPath,
                     QString *error)
{
  if (width <= 0 || height <= 0)
    return false;

  size_t required = 0;
  const int queryStatus =
      wg_psd_render_layer_required(doc, index, &required);

  /* OpenPSD returns BUFFER_TOO_SMALL for a size query.  Any positive/zero
     result with a valid required size is usable. */
  if (required == 0) {
    if (error)
      *error = QString::fromUtf8(wg_psd_status_text(queryStatus));
    return false;
  }

  const size_t expected =
      static_cast<size_t>(width) *
      static_cast<size_t>(height) * 4u;

  if (required < expected) {
    if (error)
      *error = "La capa no devolvió suficientes píxeles RGBA.";
    return false;
  }

  QByteArray rgba;
  rgba.resize(static_cast<qsizetype>(required));

  const int renderStatus =
      wg_psd_render_layer(
          doc,
          index,
          reinterpret_cast<uint8_t *>(rgba.data()),
          required);

  if (renderStatus != 0) {
    if (error)
      *error = QString::fromUtf8(wg_psd_status_text(renderStatus));
    return false;
  }

  QImage view(reinterpret_cast<const uchar *>(rgba.constData()),
              width,
              height,
              width * 4,
              QImage::Format_RGBA8888);

  const QString path =
      assetDir + "/" +
      QString("%1_%2.png")
          .arg(index, 4, 10, QLatin1Char('0'))
          .arg(safeFileName(name));

  if (!view.copy().save(path, "PNG")) {
    if (error)
      *error = "No se pudo guardar la capa rasterizada como PNG.";
    return false;
  }

  if (outPath)
    *outPath = path;
  return true;
}

bool importTextLayer(wg_psd_doc *doc,
                     int32_t index,
                     const QString &name,
                     const QString &parentId,
                     Layer *out,
                     QString *warning)
{
  if (!out)
    return false;

  Layer layer = commonLayer(doc, index, name, parentId);
  layer.type = LayerType::Text;

  QByteArray textBuffer(65536, '\0');
  const int textStatus =
      wg_psd_text(doc, index, textBuffer.data(),
                  static_cast<size_t>(textBuffer.size()));

  if (textStatus == 0)
    layer.text = QString::fromUtf8(textBuffer.constData());
  else
    layer.text = name;

  wg_psd_text_style style{};
  if (wg_psd_text_style_at(doc, index, &style) == 0) {
    if (style.font_name[0] != '\0')
      layer.fontFamily = QString::fromUtf8(style.font_name);

    if (style.size > 0.0) {
      layer.fontSize = std::clamp(
          static_cast<int>(qRound(style.size)), 6, 400);
      layer.minFontSize = std::max(6, layer.fontSize / 2);
    }

    layer.color = QColor(style.color_rgba[0],
                         style.color_rgba[1],
                         style.color_rgba[2],
                         style.color_rgba[3]);

    layer.textHorizontalAlign =
        horizontalAlignment(style.justification);
  }

  wg_psd_text_geometry geometry{};
  if (wg_psd_text_geometry_at(doc, index, &geometry) == 0) {
    const double w = geometry.right - geometry.left;
    const double h = geometry.bottom - geometry.top;

    if (w > 1.0 && h > 1.0) {
      layer.position =
          QPointF(geometry.tx + geometry.left,
                  geometry.ty + geometry.top);
      layer.size = QSizeF(w, h);
    }
  }

  layer.textVerticalAlign = TextVerticalAlign::Top;
  layer.textAutoFit = true;
  layer.textWrap = true;
  layer.splitOverflow = false;

  const int explicitLines =
      std::max(1,
               static_cast<int>(layer.text.count('\n')) + 1);
  layer.maxLines =
      std::clamp(explicitLines + 3, 1, 12);

  *out = layer;

  if (textStatus != 0 && warning) {
    *warning =
        QString("Texto «%1»: la capa se importó, pero el contenido "
                "editable completo no estuvo disponible.")
            .arg(name);
  }

  return true;
}

} // namespace

PsdImportResult PsdImporter::importFile(const QString &filePath)
{
  PsdImportResult result;

  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    result.error = "No se pudo abrir el archivo PSD/PSB.";
    return result;
  }

  const QByteArray bytes = file.readAll();
  if (bytes.isEmpty()) {
    result.error = "El archivo está vacío.";
    return result;
  }

  char openError[512] = {};
  wg_psd_doc *doc =
      wg_psd_open_memory(bytes.constData(),
                         static_cast<size_t>(bytes.size()),
                         openError,
                         sizeof(openError));

  if (!doc) {
    result.error =
        QString("OpenPSD no pudo analizar el archivo: %1")
            .arg(QString::fromUtf8(openError));
    return result;
  }

  uint32_t documentWidth = 0;
  uint32_t documentHeight = 0;
  if (wg_psd_dimensions(doc, &documentWidth,
                        &documentHeight) != 0 ||
      documentWidth == 0 || documentHeight == 0) {
    wg_psd_close(doc);
    result.error = "El PSD no tiene dimensiones válidas.";
    return result;
  }

  Project project;
  project.name = QFileInfo(filePath).completeBaseName();
  project.canvas =
      QSize(static_cast<int>(documentWidth),
            static_cast<int>(documentHeight));
  project.layers.clear();

  const QString assetDir =
      importedAssetDirectory(filePath);

  int32_t layerCount = 0;
  if (wg_psd_layer_count(doc, &layerCount) != 0) {
    wg_psd_close(doc);
    result.error = "No se pudo leer la lista de capas del PSD.";
    return result;
  }

  QVector<QString> groupStack;
  QVector<Layer> topToBottom;
  bool hasVerseField = false;
  bool hasReferenceField = false;

  for (int32_t index = layerCount - 1;
       index >= 0;
       --index) {
    int type = WG_PSD_EMPTY;
    if (wg_psd_layer_type_at(doc, index, &type) != 0)
      continue;

    if (type == WG_PSD_GROUP_END) {
      if (!groupStack.isEmpty())
        groupStack.removeLast();
      continue;
    }

    const QString name = layerName(doc, index);
    const QString parentId =
        groupStack.isEmpty()
            ? QString()
            : groupStack.constLast();

    if (type == WG_PSD_GROUP_START) {
      Layer group =
          commonLayer(doc, index, name, parentId);
      group.type = LayerType::Group;
      topToBottom.push_back(group);
      groupStack.push_back(group.id);
      ++result.groupLayers;
      continue;
    }

    if (type == WG_PSD_TEXT) {
      Layer text;
      QString warning;
      if (importTextLayer(doc, index, name,
                          parentId, &text, &warning)) {
        topToBottom.push_back(text);
        ++result.textLayers;
        if (!warning.isEmpty())
          result.warnings << warning;

        if (name == "{{VERSICULO}}")
          hasVerseField = true;
        if (name == "{{REFERENCIA}}")
          hasReferenceField = true;
      } else {
        ++result.skippedLayers;
      }
      continue;
    }

    int32_t top = 0;
    int32_t left = 0;
    int32_t bottom = 0;
    int32_t right = 0;
    wg_psd_layer_bounds(
        doc, index, &top, &left, &bottom, &right);

    const int width = right - left;
    const int height = bottom - top;

    QString pngPath;
    QString rasterError;
    if (saveRasterLayer(doc,
                        index,
                        assetDir,
                        name,
                        width,
                        height,
                        &pngPath,
                        &rasterError)) {
      Layer image =
          commonLayer(doc, index, name, parentId);
      image.type = LayerType::Image;
      image.imagePath = pngPath;
      topToBottom.push_back(image);
      ++result.imageLayers;

      if (type != WG_PSD_PIXEL) {
        result.warnings
            << QString("«%1» se convirtió a imagen para "
                       "conservar la apariencia disponible.")
                   .arg(name);
      }
    } else {
      ++result.skippedLayers;
      result.warnings
          << QString("«%1» no pudo convertirse y se omitió%2.")
                 .arg(name,
                      rasterError.isEmpty()
                          ? QString()
                          : QString(": ") + rasterError);
    }
  }

  std::reverse(topToBottom.begin(),
               topToBottom.end());
  project.layers = std::move(topToBottom);

  if (hasVerseField && hasReferenceField)
    project.usage = TemplateUsage::BibleText;

  wg_psd_close(doc);

  if (project.layers.isEmpty()) {
    result.error =
        "El PSD se abrió, pero no produjo capas compatibles.";
    return result;
  }

  if (result.skippedLayers > 0) {
    result.warnings
        << QString("%1 capa(s) avanzada(s) no pudieron "
                   "importarse de forma editable.")
               .arg(result.skippedLayers);
  }

  result.project = std::move(project);
  result.ok = true;
  return result;
}

} // namespace wg
