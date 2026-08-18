#include "template-library.hpp"
#include "graphics-renderer.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

namespace wg {

namespace {
QString defaultKey(const QString &serviceKind)
{
  return "templates/default/" + serviceKind.trimmed().toLower();
}

QString hiddenBuiltinsKey()
{
  return "templates/hiddenBuiltins";
}

const QStringList &builtinIds()
{
  static const QStringList ids = {
      "builtin:pastor", "builtin:motion", "builtin:scripture",
      "builtin:sermon", "builtin:worship", "builtin:announcement"};
  return ids;
}
}

QString TemplateLibrary::libraryPath()
{
  const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/worship-graphics/templates";
  QDir().mkpath(path);
  return path;
}

static QString safeName(QString name)
{
  name = name.trimmed();
  name.replace(QRegularExpression("[^A-Za-z0-9_\\- ]"), "");
  name.replace(' ', '_');
  if (name.isEmpty()) name = "template";
  return name;
}

QVector<TemplateEntry> TemplateLibrary::entries()
{
  QVector<TemplateEntry> result;
  QDir dir(libraryPath());
  const auto files = dir.entryInfoList({"*.wgtpl"}, QDir::Files, QDir::Time);

  for (const QFileInfo &file : files) {
    TemplateEntry entry;
    entry.filePath = file.absoluteFilePath();
    entry.name = file.completeBaseName().replace('_', ' ');
    const QString thumb = file.absolutePath() + "/" + file.completeBaseName() + ".png";
    if (QFile::exists(thumb)) entry.thumbnailPath = thumb;

    QFile jsonFile(entry.filePath);
    if (jsonFile.open(QIODevice::ReadOnly)) {
      const QJsonDocument doc = QJsonDocument::fromJson(jsonFile.readAll());
      if (doc.isObject()) {
        const QJsonObject root = doc.object();
        entry.name = root.value("name").toString(entry.name);
        entry.usage = static_cast<TemplateUsage>(root.value("usage").toInt(0));
      }
    }

    result.push_back(entry);
  }
  return result;
}

bool TemplateLibrary::save(const Project &project, const QString &name, QString *error)
{
  QJsonObject root;
  root["format"] = "WorshipGraphicsTemplate";
  root["version"] = 2;
  root["name"] = name;
  root["canvasWidth"] = project.canvas.width();
  root["canvasHeight"] = project.canvas.height();
  root["usage"] = static_cast<int>(project.usage);

  QJsonArray layers;
  for (const Layer &l : project.layers) {
    QJsonObject o;
    o["id"] = l.id; o["name"] = l.name; o["parentId"] = l.parentId;
    o["type"] = static_cast<int>(l.type); o["visible"] = l.visible; o["locked"] = l.locked;
    o["x"] = l.position.x(); o["y"] = l.position.y(); o["w"] = l.size.width(); o["h"] = l.size.height();
    o["opacity"] = l.opacity; o["rotation"] = l.rotationDeg; o["radius"] = l.cornerRadius;
    o["color"] = l.color.name(QColor::HexArgb); o["textured"] = l.textured;
    o["text"] = l.text; o["fontFamily"] = l.fontFamily; o["fontSize"] = l.fontSize; o["minFontSize"] = l.minFontSize; o["bold"] = l.bold;
    o["autoFit"] = l.textAutoFit; o["wrap"] = l.textWrap; o["splitOverflow"] = l.splitOverflow; o["maxLines"] = l.maxLines;
    o["textHAlign"] = static_cast<int>(l.textHorizontalAlign);
    o["textVAlign"] = static_cast<int>(l.textVerticalAlign);
    o["imagePath"] = l.imagePath;
    o["enter"] = static_cast<int>(l.enterAnimation); o["exit"] = static_cast<int>(l.exitAnimation);
    o["enterDelay"] = l.enterDelayMs; o["exitDelay"] = l.exitDelayMs;
    o["enterDuration"] = l.enterDurationMs; o["exitDuration"] = l.exitDurationMs;
    layers.append(o);
  }
  root["layers"] = layers;

  const QString base = libraryPath() + "/" + safeName(name);
  QFile file(base + ".wgtpl");
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    if (error) *error = "No se pudo guardar la plantilla.";
    return false;
  }
  file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  file.close();

  GraphicsRenderer::render(project)
      .scaled(480, 270, Qt::KeepAspectRatio, Qt::SmoothTransformation)
      .save(base + ".png");
  return true;
}

bool TemplateLibrary::load(const QString &filePath, Project *project, QString *error)
{
  if (!project) return false;
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    if (error) *error = "No se pudo abrir la plantilla.";
    return false;
  }
  const auto doc = QJsonDocument::fromJson(file.readAll());
  if (!doc.isObject()) {
    if (error) *error = "La plantilla no es válida.";
    return false;
  }

  const QJsonObject root = doc.object();
  Project out;
  out.name = root.value("name").toString("Plantilla");
  out.canvas = QSize(root.value("canvasWidth").toInt(1920), root.value("canvasHeight").toInt(1080));
  out.usage = static_cast<TemplateUsage>(root.value("usage").toInt(0));

  const auto layers = root.value("layers").toArray();
  for (const auto &value : layers) {
    const auto o = value.toObject();
    Layer l;
    l.id = o.value("id").toString(); l.name = o.value("name").toString(); l.parentId = o.value("parentId").toString();
    l.type = static_cast<LayerType>(o.value("type").toInt()); l.visible = o.value("visible").toBool(true); l.locked = o.value("locked").toBool(false);
    l.position = {o.value("x").toDouble(), o.value("y").toDouble()};
    l.size = {o.value("w").toDouble(400), o.value("h").toDouble(120)};
    l.opacity = o.value("opacity").toDouble(1.0); l.rotationDeg = o.value("rotation").toDouble(); l.cornerRadius = o.value("radius").toDouble(18.0);
    l.color = QColor(o.value("color").toString("#FFFFFFFF")); l.textured = o.value("textured").toBool(false);
    l.text = o.value("text").toString(); l.fontFamily = o.value("fontFamily").toString("Segoe UI"); l.fontSize = o.value("fontSize").toInt(48); l.minFontSize = o.value("minFontSize").toInt(qMin(24, l.fontSize)); l.bold = o.value("bold").toBool(false);
    l.textAutoFit = o.value("autoFit").toBool(true); l.textWrap = o.value("wrap").toBool(true); l.splitOverflow = o.value("splitOverflow").toBool(true); l.maxLines = o.value("maxLines").toInt(2);
    l.textHorizontalAlign = static_cast<TextHorizontalAlign>(o.value("textHAlign").toInt(static_cast<int>(TextHorizontalAlign::Left)));
    l.textVerticalAlign = static_cast<TextVerticalAlign>(o.value("textVAlign").toInt(static_cast<int>(TextVerticalAlign::Middle)));
    l.imagePath = o.value("imagePath").toString();
    l.enterAnimation = static_cast<AnimationPreset>(o.value("enter").toInt()); l.exitAnimation = static_cast<AnimationPreset>(o.value("exit").toInt());
    l.enterDelayMs = o.value("enterDelay").toInt(); l.exitDelayMs = o.value("exitDelay").toInt();
    const int legacyDuration = o.value("duration").toInt(450);
    l.enterDurationMs = o.value("enterDuration").toInt(legacyDuration);
    l.exitDurationMs = o.value("exitDuration").toInt(legacyDuration);
    out.layers.push_back(l);
  }
  *project = std::move(out);
  return true;
}

bool TemplateLibrary::remove(const QString &filePath, QString *error)
{
  const QFileInfo info(filePath);
  if (!info.exists() || info.suffix().compare("wgtpl", Qt::CaseInsensitive) != 0) {
    if (error) *error = "La plantilla seleccionada no existe.";
    return false;
  }

  const QString expectedDir = QDir::cleanPath(QFileInfo(libraryPath()).absoluteFilePath());
  const QString actualDir = QDir::cleanPath(info.absolutePath());
  if (QString::compare(expectedDir, actualDir, Qt::CaseInsensitive) != 0) {
    if (error) *error = "Solo se pueden eliminar plantillas personales de Worship Graphics.";
    return false;
  }

  const QString thumb = info.absolutePath() + "/" + info.completeBaseName() + ".png";
  if (!QFile::remove(info.absoluteFilePath())) {
    if (error) *error = "No se pudo eliminar la plantilla.";
    return false;
  }
  if (QFile::exists(thumb)) QFile::remove(thumb);

  QSettings settings("WorshipGraphics", "WorshipGraphics");
  const QStringList kinds = {"pastor", "scripture", "sermon", "worship", "announcement"};
  for (const QString &kind : kinds) {
    const QString key = defaultKey(kind);
    if (settings.value(key).toString() == info.absoluteFilePath())
      settings.remove(key);
  }
  settings.sync();
  return true;
}

bool TemplateLibrary::removeOrHide(const QString &templateId, QString *error)
{
  if (templateId.startsWith("builtin:")) {
    if (!builtinIds().contains(templateId)) {
      if (error) *error = "La plantilla integrada no es válida.";
      return false;
    }

    QSettings settings("WorshipGraphics", "WorshipGraphics");
    QStringList hidden = settings.value(hiddenBuiltinsKey()).toStringList();
    if (!hidden.contains(templateId)) hidden.push_back(templateId);
    settings.setValue(hiddenBuiltinsKey(), hidden);

    const QStringList kinds = {"pastor", "scripture", "sermon", "worship", "announcement"};
    for (const QString &kind : kinds) {
      const QString key = defaultKey(kind);
      if (settings.value(key).toString() == templateId)
        settings.remove(key);
    }
    settings.sync();
    return true;
  }

  return remove(templateId, error);
}

bool TemplateLibrary::isBuiltinHidden(const QString &templateId)
{
  if (!templateId.startsWith("builtin:")) return false;
  QSettings settings("WorshipGraphics", "WorshipGraphics");
  return settings.value(hiddenBuiltinsKey()).toStringList().contains(templateId);
}

void TemplateLibrary::restoreBuiltins()
{
  QSettings settings("WorshipGraphics", "WorshipGraphics");
  settings.remove(hiddenBuiltinsKey());
  settings.sync();
}

QString TemplateLibrary::displayNameForId(const QString &templateId)
{
  if (templateId == "builtin:pastor") return "Pastor Clean";
  if (templateId == "builtin:motion") return "Motion Pieces";
  if (templateId == "builtin:scripture") return "Versículo";
  if (templateId == "builtin:sermon") return "Tema de prédica";
  if (templateId == "builtin:worship") return "Alabanza";
  if (templateId == "builtin:announcement") return "Anuncio";

  if (!templateId.isEmpty()) {
    Project p;
    if (load(templateId, &p, nullptr))
      return p.name;
  }
  return QString();
}

bool TemplateLibrary::isValidTemplateId(const QString &templateId)
{
  if (templateId.startsWith("builtin:"))
    return builtinIds().contains(templateId);
  if (templateId.isEmpty()) return false;
  const QFileInfo info(templateId);
  return info.exists() && info.isFile() && info.suffix().compare("wgtpl", Qt::CaseInsensitive) == 0;
}


bool TemplateLibrary::isBibleTemplateId(const QString &templateId)
{
  if (templateId == "builtin:scripture")
    return !isBuiltinHidden(templateId);
  if (!isValidTemplateId(templateId) || templateId.startsWith("builtin:"))
    return false;

  Project project;
  if (!load(templateId, &project, nullptr) || project.usage != TemplateUsage::BibleText)
    return false;

  bool verse = false;
  bool reference = false;
  for (const Layer &layer : project.layers) {
    if (layer.name == "{{VERSICULO}}") verse = true;
    if (layer.name == "{{REFERENCIA}}") reference = true;
  }
  return verse && reference;
}

void TemplateLibrary::setBibleDefaultTemplate(const QString &templateId)
{
  if (isBibleTemplateId(templateId))
    setDefaultTemplate("scripture", templateId);
}

QString TemplateLibrary::preferredBibleTemplate()
{
  QString id = defaultTemplate("scripture");
  if (!id.isEmpty() && isBibleTemplateId(id))
    return id;

  for (const TemplateEntry &entry : entries()) {
    if (entry.usage == TemplateUsage::BibleText && isBibleTemplateId(entry.filePath)) {
      setDefaultTemplate("scripture", entry.filePath);
      return entry.filePath;
    }
  }

  if (!isBuiltinHidden("builtin:scripture")) {
    setDefaultTemplate("scripture", "builtin:scripture");
    return "builtin:scripture";
  }

  return QString();
}

void TemplateLibrary::setDefaultTemplate(const QString &serviceKind, const QString &templateId)
{
  QSettings settings("WorshipGraphics", "WorshipGraphics");
  const QString key = defaultKey(serviceKind);
  if (!isValidTemplateId(templateId))
    settings.remove(key);
  else {
    if (templateId.startsWith("builtin:")) {
      QStringList hidden = settings.value(hiddenBuiltinsKey()).toStringList();
      hidden.removeAll(templateId);
      settings.setValue(hiddenBuiltinsKey(), hidden);
    }
    settings.setValue(key, templateId);
  }
  settings.sync();
}

QString TemplateLibrary::defaultTemplate(const QString &serviceKind)
{
  QSettings settings("WorshipGraphics", "WorshipGraphics");
  const QString key = defaultKey(serviceKind);
  const QString id = settings.value(key).toString();
  if (id.isEmpty()) return QString();
  if (!isValidTemplateId(id)) {
    settings.remove(key);
    settings.sync();
    return QString();
  }
  return id;
}

} // namespace wg
