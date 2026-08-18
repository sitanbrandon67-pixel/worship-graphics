#include "design-page.hpp"
#include "app-state.hpp"
#include "graphics-renderer.hpp"
#include "template-factory.hpp"
#include "template-library.hpp"
#include "theme.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QPixmap>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QUuid>

namespace wg {

static QStringList animationNames()
{
  return {"Ninguna", "Fade", "Desde izquierda", "Desde derecha", "Desde arriba", "Desde abajo", "Zoom", "Pop", "Expandir horizontal", "Expandir vertical"};
}

static QIcon projectIcon(const Project &project)
{
  const QImage img = GraphicsRenderer::render(project).scaled(180, 102, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  return QIcon(QPixmap::fromImage(img));
}

static Layer *currentLayer(Project &project, int row)
{
  if (row < 0 || row >= project.layers.size()) return nullptr;
  return &project.layers[row];
}

DesignPage::DesignPage(QWidget *parent) : QWidget(parent)
{
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(16, 14, 16, 14);
  root->setSpacing(10);

  auto *libraryCard = new QFrame();
  libraryCard->setObjectName("wgCard");
  applySoftShadow(libraryCard);
  auto *libraryLayout = new QVBoxLayout(libraryCard);
  auto *libraryTitle = new QHBoxLayout();
  auto *title = new QLabel("BIBLIOTECA DE PLANTILLAS"); title->setObjectName("wgSectionTitle");
  auto *save = new QPushButton("GUARDAR PLANTILLA"); save->setObjectName("wgSoftButton");
  libraryTitle->addWidget(title); libraryTitle->addStretch(); libraryTitle->addWidget(save);
  libraryLayout->addLayout(libraryTitle);
  templates_ = new QListWidget();
  templates_->setObjectName("wgTemplateLibrary");
  templates_->setViewMode(QListView::IconMode);
  templates_->setResizeMode(QListView::Adjust);
  templates_->setMovement(QListView::Static);
  templates_->setIconSize({160, 90});
  templates_->setSpacing(10);
  templates_->setFixedHeight(132);
  libraryLayout->addWidget(templates_);
  root->addWidget(libraryCard);

  auto *toolbar = new QHBoxLayout();
  auto *importPsd = new QPushButton("IMPORTAR PSD");
  auto *addText = new QPushButton("+ TEXTO");
  auto *addShape = new QPushButton("+ FORMA");
  auto *addImage = new QPushButton("+ IMAGEN");
  auto *duplicate = new QPushButton("DUPLICAR");
  auto *remove = new QPushButton("ELIMINAR"); remove->setObjectName("wgDanger");
  auto *stagger = new QPushButton("ESCALONAR 80ms");
  toolbar->addWidget(importPsd); toolbar->addWidget(addText); toolbar->addWidget(addShape); toolbar->addWidget(addImage);
  toolbar->addSpacing(10); toolbar->addWidget(duplicate); toolbar->addWidget(remove); toolbar->addStretch(); toolbar->addWidget(stagger);
  root->addLayout(toolbar);

  auto *body = new QHBoxLayout();
  body->setSpacing(10);

  auto *layersCard = new QFrame(); layersCard->setObjectName("wgCard"); layersCard->setFixedWidth(280); applySoftShadow(layersCard);
  auto *layersLayout = new QVBoxLayout(layersCard);
  auto *layersTitle = new QLabel("CAPAS"); layersTitle->setObjectName("wgSectionTitle");
  layersLayout->addWidget(layersTitle);
  layers_ = new QListWidget();
  layersLayout->addWidget(layers_);
  auto *layerButtons = new QHBoxLayout();
  auto *up = new QPushButton("↑"); auto *down = new QPushButton("↓");
  auto *show = new QPushButton("VER"); auto *lock = new QPushButton("BLOQ");
  layerButtons->addWidget(up); layerButtons->addWidget(down); layerButtons->addWidget(show); layerButtons->addWidget(lock);
  layersLayout->addLayout(layerButtons);
  body->addWidget(layersCard);

  auto *center = new QVBoxLayout();
  auto *canvasCard = new QFrame(); canvasCard->setObjectName("wgCard"); applySoftShadow(canvasCard);
  auto *canvasLayout = new QVBoxLayout(canvasCard);
  auto *canvasTitle = new QLabel("CANVAS"); canvasTitle->setObjectName("wgSectionTitle");
  canvasLayout->addWidget(canvasTitle);
  canvas_ = new QLabel(); canvas_->setObjectName("wgScreen"); canvas_->setAlignment(Qt::AlignCenter); canvas_->setMinimumHeight(360);
  canvasLayout->addWidget(canvas_);
  center->addWidget(canvasCard, 1);
  body->addLayout(center, 1);

  auto *propsCard = new QFrame(); propsCard->setObjectName("wgCard"); propsCard->setFixedWidth(340); applySoftShadow(propsCard);
  auto *propsLayout = new QVBoxLayout(propsCard);
  auto *propsTitle = new QLabel("PROPIEDADES"); propsTitle->setObjectName("wgSectionTitle");
  propsLayout->addWidget(propsTitle);

  bibleTemplate_ = new QCheckBox("Plantilla bíblica");
  propsLayout->addWidget(bibleTemplate_);

  auto *form = new QFormLayout();
  name_ = new QLineEdit(); text_ = new QLineEdit(); color_ = new QLineEdit("#FFFFFFFF");
  x_ = new QSpinBox(); y_ = new QSpinBox(); w_ = new QSpinBox(); h_ = new QSpinBox();
  fontSize_ = new QSpinBox(); minFontSize_ = new QSpinBox(); maxLines_ = new QSpinBox();
  opacity_ = new QSpinBox(); radius_ = new QSpinBox(); rotation_ = new QDoubleSpinBox();
  enterDelay_ = new QSpinBox(); exitDelay_ = new QSpinBox(); duration_ = new QSpinBox();
  enterAnimation_ = new QComboBox(); exitAnimation_ = new QComboBox();
  textAlignH_ = new QComboBox(); textAlignV_ = new QComboBox();
  autoFit_ = new QCheckBox("Auto ajustar al marco"); wrap_ = new QCheckBox("Respetar marco / saltos"); splitOverflow_ = new QCheckBox("Dividir en dos líneas si excede");

  for (QSpinBox *box : {x_, y_, w_, h_, fontSize_, minFontSize_, maxLines_, opacity_, radius_, enterDelay_, exitDelay_, duration_}) {
    box->setRange(0, 5000);
  }
  x_->setRange(-5000, 5000); y_->setRange(-5000, 5000);
  opacity_->setRange(0, 100);
  fontSize_->setRange(6, 400); minFontSize_->setRange(6, 400); maxLines_->setRange(1, 6);
  radius_->setRange(0, 500);
  rotation_->setRange(-360.0, 360.0);
  enterAnimation_->addItems(animationNames());
  exitAnimation_->addItems(animationNames());
  textAlignH_->addItems({"Izquierda", "Centro", "Derecha"});
  textAlignV_->addItems({"Arriba", "Centro", "Abajo"});

  form->addRow("Nombre", name_);
  form->addRow("Texto", text_);
  form->addRow("Color", color_);
  form->addRow("X", x_);
  form->addRow("Y", y_);
  form->addRow("Ancho", w_);
  form->addRow("Alto", h_);
  form->addRow("Fuente", fontSize_);
  form->addRow("Mín. fuente", minFontSize_);
  form->addRow("Máx. líneas", maxLines_);
  form->addRow("Opacidad %", opacity_);
  form->addRow("Esquina", radius_);
  form->addRow("Rotación", rotation_);
  form->addRow("Delay entrada", enterDelay_);
  form->addRow("Delay salida", exitDelay_);
  form->addRow("Duración", duration_);
  form->addRow("Anim. entrada", enterAnimation_);
  form->addRow("Anim. salida", exitAnimation_);
  form->addRow("Alineación H", textAlignH_);
  form->addRow("Alineación V", textAlignV_);
  propsLayout->addLayout(form);
  propsLayout->addWidget(autoFit_);
  propsLayout->addWidget(wrap_);
  propsLayout->addWidget(splitOverflow_);

  auto *fieldButtons = new QHBoxLayout();
  auto *verseField = new QPushButton("CAMPO VERSÍCULO");
  auto *refField = new QPushButton("CAMPO REFERENCIA");
  fieldButtons->addWidget(verseField); fieldButtons->addWidget(refField);
  propsLayout->addLayout(fieldButtons);

  auto *alignLabel = new QLabel("ALINEAR CAPA EN CANVAS"); alignLabel->setObjectName("wgSectionTitle");
  propsLayout->addWidget(alignLabel);
  auto *align1 = new QHBoxLayout();
  auto *left = new QPushButton("Izq"); auto *centerH = new QPushButton("Centro H"); auto *right = new QPushButton("Der");
  align1->addWidget(left); align1->addWidget(centerH); align1->addWidget(right);
  propsLayout->addLayout(align1);
  auto *align2 = new QHBoxLayout();
  auto *top = new QPushButton("Arriba"); auto *centerV = new QPushButton("Centro V"); auto *bottom = new QPushButton("Abajo");
  align2->addWidget(top); align2->addWidget(centerV); align2->addWidget(bottom);
  propsLayout->addLayout(align2);

  auto *apply = new QPushButton("APLICAR CAMBIOS"); apply->setObjectName("wgPrimary");
  propsLayout->addWidget(apply);
  propsLayout->addStretch(1);
  body->addWidget(propsCard);

  root->addLayout(body, 1);

  connect(addText, &QPushButton::clicked, this, &DesignPage::addTextLayer);
  connect(addShape, &QPushButton::clicked, this, &DesignPage::addShapeLayer);
  connect(addImage, &QPushButton::clicked, this, &DesignPage::addImageLayer);
  connect(remove, &QPushButton::clicked, this, &DesignPage::deleteCurrentLayer);
  connect(duplicate, &QPushButton::clicked, this, &DesignPage::duplicateCurrentLayer);
  connect(up, &QPushButton::clicked, this, [this]{ moveCurrentLayer(-1); });
  connect(down, &QPushButton::clicked, this, [this]{ moveCurrentLayer(1); });
  connect(show, &QPushButton::clicked, this, &DesignPage::toggleVisibility);
  connect(lock, &QPushButton::clicked, this, &DesignPage::toggleLock);
  connect(stagger, &QPushButton::clicked, this, &DesignPage::applyStagger);
  connect(importPsd, &QPushButton::clicked, this, &DesignPage::importPsdPlaceholder);
  connect(save, &QPushButton::clicked, this, &DesignPage::saveTemplate);
  connect(templates_, &QListWidget::itemDoubleClicked, this, [this]{ loadSelectedTemplate(); });
  connect(layers_, &QListWidget::currentRowChanged, this, &DesignPage::selectLayer);
  connect(apply, &QPushButton::clicked, this, &DesignPage::applyProperties);
  connect(bibleTemplate_, &QCheckBox::toggled, this, &DesignPage::markBibleTemplate);
  connect(verseField, &QPushButton::clicked, this, &DesignPage::markAsVerseField);
  connect(refField, &QPushButton::clicked, this, &DesignPage::markAsReferenceField);
  connect(left, &QPushButton::clicked, this, &DesignPage::alignLayerLeft);
  connect(centerH, &QPushButton::clicked, this, &DesignPage::alignLayerCenterH);
  connect(right, &QPushButton::clicked, this, &DesignPage::alignLayerRight);
  connect(top, &QPushButton::clicked, this, &DesignPage::alignLayerTop);
  connect(centerV, &QPushButton::clicked, this, &DesignPage::alignLayerCenterV);
  connect(bottom, &QPushButton::clicked, this, &DesignPage::alignLayerBottom);

  refreshTemplateLibrary();
  rebuildLayerList();
  refreshCanvas();
}

void DesignPage::rebuildLayerList()
{
  layers_->clear();
  const auto &project = AppState::instance().project();
  for (int i = 0; i < project.layers.size(); ++i) {
    const Layer &l = project.layers[i];
    QString label = l.name;
    if (l.type == LayerType::Text)
      label += " · T";
    if (l.type == LayerType::Shape)
      label += " · F";
    if (l.type == LayerType::Image)
      label += " · IMG";
    if (!l.visible)
      label += " · oculta";
    layers_->addItem(label);
  }
  setCurrentRowSafe(currentRow_ < 0 ? 0 : currentRow_);
}

void DesignPage::refreshTemplateLibrary()
{
  templates_->clear();
  auto defaults = QVector<Project>{TemplateFactory::pastorLowerThird(), TemplateFactory::motionPiecesLowerThird(), TemplateFactory::scriptureLowerThird()};
  for (const auto &project : defaults) {
    auto *item = new QListWidgetItem(projectIcon(project), project.name);
    item->setData(Qt::UserRole, QString());
    templates_->addItem(item);
  }
  for (const auto &entry : TemplateLibrary::entries()) {
    auto *item = new QListWidgetItem(entry.thumbnailPath.isEmpty() ? QIcon() : QIcon(entry.thumbnailPath), entry.name);
    item->setData(Qt::UserRole, entry.filePath);
    templates_->addItem(item);
  }
}

void DesignPage::selectLayer(int row)
{
  currentRow_ = row;
  auto &project = AppState::instance().mutableProject();
  auto *layer = currentLayer(project, row);
  if (!layer)
    return;

  name_->setText(layer->name);
  text_->setText(layer->text);
  color_->setText(layer->color.name(QColor::HexArgb));
  x_->setValue(qRound(layer->position.x()));
  y_->setValue(qRound(layer->position.y()));
  w_->setValue(qRound(layer->size.width()));
  h_->setValue(qRound(layer->size.height()));
  fontSize_->setValue(layer->fontSize);
  minFontSize_->setValue(layer->minFontSize);
  maxLines_->setValue(layer->maxLines);
  opacity_->setValue(qRound(layer->opacity * 100.0));
  radius_->setValue(qRound(layer->cornerRadius));
  rotation_->setValue(layer->rotationDeg);
  enterDelay_->setValue(layer->enterDelayMs);
  exitDelay_->setValue(layer->exitDelayMs);
  duration_->setValue(layer->animationDurationMs);
  enterAnimation_->setCurrentIndex(static_cast<int>(layer->enterAnimation));
  exitAnimation_->setCurrentIndex(static_cast<int>(layer->exitAnimation));
  textAlignH_->setCurrentIndex(static_cast<int>(layer->textHorizontalAlign));
  textAlignV_->setCurrentIndex(static_cast<int>(layer->textVerticalAlign));
  autoFit_->setChecked(layer->textAutoFit);
  wrap_->setChecked(layer->textWrap);
  splitOverflow_->setChecked(layer->splitOverflow);
  bibleTemplate_->setChecked(AppState::instance().project().usage == TemplateUsage::BibleText);
}

void DesignPage::applyProperties()
{
  auto &project = AppState::instance().mutableProject();
  auto *layer = currentLayer(project, currentRow_);
  if (!layer)
    return;

  layer->name = name_->text().trimmed();
  layer->text = text_->text();
  layer->color = QColor(color_->text().trimmed());
  if (!layer->color.isValid()) layer->color = QColor("#FFFFFFFF");
  layer->position = {double(x_->value()), double(y_->value())};
  layer->size = {double(w_->value()), double(h_->value())};
  layer->fontSize = fontSize_->value();
  layer->minFontSize = qMin(fontSize_->value(), minFontSize_->value());
  layer->maxLines = maxLines_->value();
  layer->opacity = opacity_->value() / 100.0;
  layer->cornerRadius = radius_->value();
  layer->rotationDeg = rotation_->value();
  layer->enterDelayMs = enterDelay_->value();
  layer->exitDelayMs = exitDelay_->value();
  layer->animationDurationMs = duration_->value();
  layer->enterAnimation = static_cast<AnimationPreset>(enterAnimation_->currentIndex());
  layer->exitAnimation = static_cast<AnimationPreset>(exitAnimation_->currentIndex());
  layer->textHorizontalAlign = static_cast<TextHorizontalAlign>(textAlignH_->currentIndex());
  layer->textVerticalAlign = static_cast<TextVerticalAlign>(textAlignV_->currentIndex());
  layer->textAutoFit = autoFit_->isChecked();
  layer->textWrap = wrap_->isChecked();
  layer->splitOverflow = splitOverflow_->isChecked();

  AppState::instance().notifyModelChanged();
  rebuildLayerList();
  refreshCanvas();
}

void DesignPage::addTextLayer()
{
  Layer l;
  l.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  l.name = "Texto";
  l.type = LayerType::Text;
  l.position = {160, 850};
  l.size = {680, 90};
  l.text = "NUEVO TEXTO";
  AppState::instance().mutableProject().layers.push_back(l);
  AppState::instance().notifyModelChanged();
  rebuildLayerList();
  setCurrentRowSafe(AppState::instance().project().layers.size() - 1);
}

void DesignPage::addShapeLayer()
{
  Layer l;
  l.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  l.name = "Forma";
  l.type = LayerType::Shape;
  l.position = {120, 820};
  l.size = {400, 120};
  l.color = QColor("#444444");
  AppState::instance().mutableProject().layers.push_back(l);
  AppState::instance().notifyModelChanged();
  rebuildLayerList();
  setCurrentRowSafe(AppState::instance().project().layers.size() - 1);
}

void DesignPage::addImageLayer()
{
  const QString file = QFileDialog::getOpenFileName(this, "Elegir imagen", {}, "Imágenes (*.png *.jpg *.jpeg *.webp)");
  if (file.isEmpty()) return;
  Layer l;
  l.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  l.name = "Imagen";
  l.type = LayerType::Image;
  l.position = {120, 760};
  l.size = {360, 180};
  l.imagePath = file;
  AppState::instance().mutableProject().layers.push_back(l);
  AppState::instance().notifyModelChanged();
  rebuildLayerList();
  setCurrentRowSafe(AppState::instance().project().layers.size() - 1);
}

void DesignPage::deleteCurrentLayer()
{
  if (currentRow_ < 0) return;
  AppState::instance().removeLayer(currentRow_);
  if (currentRow_ >= AppState::instance().project().layers.size())
    currentRow_ = AppState::instance().project().layers.size() - 1;
  rebuildLayerList();
  refreshCanvas();
}

void DesignPage::duplicateCurrentLayer() { if (currentRow_ >= 0) AppState::instance().duplicateLayer(currentRow_); rebuildLayerList(); refreshCanvas(); }
void DesignPage::moveCurrentLayer(int delta) { if (currentRow_ >= 0) AppState::instance().moveLayer(currentRow_, delta); currentRow_ += delta; rebuildLayerList(); refreshCanvas(); }
void DesignPage::toggleVisibility() { if (currentRow_ >= 0) AppState::instance().toggleLayerVisible(currentRow_); rebuildLayerList(); refreshCanvas(); }
void DesignPage::toggleLock() { if (currentRow_ >= 0) AppState::instance().toggleLayerLocked(currentRow_); rebuildLayerList(); refreshCanvas(); }
void DesignPage::applyStagger() { AppState::instance().staggerLayers(); rebuildLayerList(); refreshCanvas(); }
void DesignPage::importPsdPlaceholder() { QMessageBox::information(this, "PSD", "La importación PSD por capas sigue disponible como flujo de trabajo, pero esta actualización se enfoca en la plantilla bíblica y el Smart Text Layout."); }

void DesignPage::saveTemplate()
{
  bool ok = false;
  const QString name = QInputDialog::getText(this, "Guardar plantilla", "Nombre de plantilla", QLineEdit::Normal, AppState::instance().project().name, &ok);
  if (!ok || name.trimmed().isEmpty()) return;
  QString error;
  if (!TemplateLibrary::save(AppState::instance().project(), name, &error)) {
    QMessageBox::warning(this, "Plantillas", error);
    return;
  }
  refreshTemplateLibrary();
}

void DesignPage::loadSelectedTemplate()
{
  const auto *item = templates_->currentItem();
  if (!item) return;
  const QString filePath = item->data(Qt::UserRole).toString();
  if (filePath.isEmpty()) {
    const QString label = item->text();
    if (label.contains("Motion")) AppState::instance().loadMotionTemplate();
    else if (label.contains("Versículo")) AppState::instance().loadScriptureTemplate("Porque de tal manera amó Dios al mundo...", "Juan 3:16");
    else AppState::instance().resetDemoProject();
  } else {
    Project project;
    QString error;
    if (!TemplateLibrary::load(filePath, &project, &error)) {
      QMessageBox::warning(this, "Plantillas", error);
      return;
    }
    AppState::instance().loadProject(project);
  }
  rebuildLayerList();
  refreshCanvas();
}

void DesignPage::markBibleTemplate(bool checked)
{
  AppState::instance().mutableProject().usage = checked ? TemplateUsage::BibleText : TemplateUsage::Generic;
  AppState::instance().notifyModelChanged();
}

void DesignPage::setLayerName(const QString &name)
{
  auto &project = AppState::instance().mutableProject();
  auto *layer = currentLayer(project, currentRow_);
  if (!layer || layer->type != LayerType::Text) return;
  layer->name = name;
  AppState::instance().notifyModelChanged();
  rebuildLayerList();
  setCurrentRowSafe(currentRow_);
}

bool DesignPage::hasCurrentTextLayer() const
{
  const auto &project = AppState::instance().project();
  return currentRow_ >= 0 && currentRow_ < project.layers.size() && project.layers[currentRow_].type == LayerType::Text;
}

void DesignPage::markAsVerseField() { if (hasCurrentTextLayer()) setLayerName("{{VERSICULO}}"); }
void DesignPage::markAsReferenceField() { if (hasCurrentTextLayer()) setLayerName("{{REFERENCIA}}"); }

void DesignPage::alignLayerLeft()
{
  auto &project = AppState::instance().mutableProject(); auto *layer = currentLayer(project, currentRow_); if (!layer) return;
  layer->position.setX(0); AppState::instance().notifyModelChanged(); selectLayer(currentRow_);
}
void DesignPage::alignLayerCenterH()
{
  auto &project = AppState::instance().mutableProject(); auto *layer = currentLayer(project, currentRow_); if (!layer) return;
  layer->position.setX((project.canvas.width() - layer->size.width()) / 2.0); AppState::instance().notifyModelChanged(); selectLayer(currentRow_);
}
void DesignPage::alignLayerRight()
{
  auto &project = AppState::instance().mutableProject(); auto *layer = currentLayer(project, currentRow_); if (!layer) return;
  layer->position.setX(project.canvas.width() - layer->size.width()); AppState::instance().notifyModelChanged(); selectLayer(currentRow_);
}
void DesignPage::alignLayerTop()
{
  auto &project = AppState::instance().mutableProject(); auto *layer = currentLayer(project, currentRow_); if (!layer) return;
  layer->position.setY(0); AppState::instance().notifyModelChanged(); selectLayer(currentRow_);
}
void DesignPage::alignLayerCenterV()
{
  auto &project = AppState::instance().mutableProject(); auto *layer = currentLayer(project, currentRow_); if (!layer) return;
  layer->position.setY((project.canvas.height() - layer->size.height()) / 2.0); AppState::instance().notifyModelChanged(); selectLayer(currentRow_);
}
void DesignPage::alignLayerBottom()
{
  auto &project = AppState::instance().mutableProject(); auto *layer = currentLayer(project, currentRow_); if (!layer) return;
  layer->position.setY(project.canvas.height() - layer->size.height()); AppState::instance().notifyModelChanged(); selectLayer(currentRow_);
}

void DesignPage::refreshCanvas()
{
  const QImage frame = AppState::instance().previewFrame();
  if (frame.isNull()) { canvas_->clear(); return; }
  canvas_->setPixmap(QPixmap::fromImage(frame).scaled(canvas_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void DesignPage::setCurrentRowSafe(int row)
{
  if (layers_->count() == 0) return;
  row = qBound(0, row, layers_->count() - 1);
  layers_->setCurrentRow(row);
}

} // namespace wg
