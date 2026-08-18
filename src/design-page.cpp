#include "design-page.hpp"
#include "app-state.hpp"
#include "graphics-renderer.hpp"
#include "template-factory.hpp"
#include "template-library.hpp"
#include "theme.hpp"
#include "timeline-widget.hpp"

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
#include <algorithm>

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

DesignPage::DesignPage(QWidget *parent) : QWidget(parent)
{
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(20, 18, 20, 18);
  root->setSpacing(12);

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
  body->setSpacing(12);

  auto *layersCard = new QFrame(); layersCard->setObjectName("wgCard"); layersCard->setFixedWidth(285); applySoftShadow(layersCard);
  auto *layersLayout = new QVBoxLayout(layersCard);
  auto *layersTitle = new QLabel("CAPAS"); layersTitle->setObjectName("wgSectionTitle"); layersLayout->addWidget(layersTitle);
  layers_ = new QListWidget();
  layers_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  layersLayout->addWidget(layers_, 1);
  auto *layerActions1 = new QHBoxLayout();
  auto *up = new QPushButton("↑"); auto *down = new QPushButton("↓"); auto *visible = new QPushButton("VISIBILIDAD"); auto *lock = new QPushButton("BLOQUEAR");
  layerActions1->addWidget(up); layerActions1->addWidget(down); layerActions1->addWidget(visible); layerActions1->addWidget(lock);
  layersLayout->addLayout(layerActions1);
  auto *layerActions2 = new QHBoxLayout();
  auto *group = new QPushButton("AGRUPAR"); auto *ungroup = new QPushButton("DESAGRUPAR");
  layerActions2->addWidget(group); layerActions2->addWidget(ungroup); layersLayout->addLayout(layerActions2);
  body->addWidget(layersCard);

  auto *canvasCard = new QFrame(); canvasCard->setObjectName("wgCard"); applySoftShadow(canvasCard);
  auto *canvasLayout = new QVBoxLayout(canvasCard);
  auto *canvasHeader = new QHBoxLayout();
  auto *canvasTitle = new QLabel("CANVAS · PREVISUALIZACIÓN DE TIEMPO"); canvasTitle->setObjectName("wgSectionTitle");
  auto *canvasInfo = new QLabel("1920×1080 · ARRASTRA EL PLAYHEAD"); canvasInfo->setObjectName("wgSubtle");
  canvasHeader->addWidget(canvasTitle); canvasHeader->addStretch(); canvasHeader->addWidget(canvasInfo); canvasLayout->addLayout(canvasHeader);
  canvas_ = new QLabel(); canvas_->setObjectName("wgScreen"); canvas_->setAlignment(Qt::AlignCenter); canvas_->setMinimumSize(500, 281);
  canvasLayout->addWidget(canvas_, 1);
  body->addWidget(canvasCard, 1);

  auto *propertiesCard = new QFrame(); propertiesCard->setObjectName("wgCard"); propertiesCard->setFixedWidth(320); applySoftShadow(propertiesCard);
  auto *propertiesLayout = new QVBoxLayout(propertiesCard);
  auto *propTitle = new QLabel("PROPIEDADES"); propTitle->setObjectName("wgSectionTitle"); propertiesLayout->addWidget(propTitle);
  auto *form = new QFormLayout();
  name_ = new QLineEdit(); text_ = new QLineEdit(); color_ = new QLineEdit();
  x_ = new QSpinBox(); y_ = new QSpinBox(); w_ = new QSpinBox(); h_ = new QSpinBox(); fontSize_ = new QSpinBox();
  opacity_ = new QSpinBox(); radius_ = new QSpinBox(); rotation_ = new QDoubleSpinBox();
  enterDelay_ = new QSpinBox(); exitDelay_ = new QSpinBox(); enterDuration_ = new QSpinBox(); exitDuration_ = new QSpinBox();
  for (auto *spin : {x_, y_, w_, h_}) spin->setRange(-8000, 8000);
  fontSize_->setRange(8, 400); opacity_->setRange(0, 100); radius_->setRange(0, 300); rotation_->setRange(-360, 360); rotation_->setDecimals(1);
  enterDelay_->setRange(0, 30000); exitDelay_->setRange(0, 30000); enterDuration_->setRange(80, 30000); exitDuration_->setRange(80, 30000);
  enterAnimation_ = new QComboBox(); exitAnimation_ = new QComboBox(); enterAnimation_->addItems(animationNames()); exitAnimation_->addItems(animationNames());
  form->addRow("Nombre", name_); form->addRow("Texto", text_); form->addRow("Color", color_);
  form->addRow("X", x_); form->addRow("Y", y_); form->addRow("Ancho", w_); form->addRow("Alto", h_);
  form->addRow("Fuente", fontSize_); form->addRow("Opacidad %", opacity_); form->addRow("Radio", radius_); form->addRow("Rotación", rotation_);
  form->addRow("Entrada", enterAnimation_); form->addRow("Delay entrada", enterDelay_); form->addRow("Duración entrada", enterDuration_);
  form->addRow("Salida", exitAnimation_); form->addRow("Delay salida", exitDelay_); form->addRow("Duración salida", exitDuration_);
  propertiesLayout->addLayout(form);
  auto *apply = new QPushButton("APLICAR CAMBIOS"); apply->setObjectName("wgPrimary"); propertiesLayout->addWidget(apply); propertiesLayout->addStretch();
  body->addWidget(propertiesCard);
  root->addLayout(body, 1);

  auto *timelineCard = new QFrame(); timelineCard->setObjectName("wgCard"); applySoftShadow(timelineCard);
  auto *timelineLayout = new QVBoxLayout(timelineCard); timelineLayout->setContentsMargins(10, 10, 10, 10); timelineLayout->setSpacing(8);
  auto *timelineControls = new QHBoxLayout();
  auto *phase = new QComboBox(); phase->addItems({"ENTRADA", "SALIDA"});
  auto *play = new QPushButton("▶ REPRODUCIR"); play->setObjectName("wgPrimary");
  auto *stop = new QPushButton("■ DETENER");
  auto *speedLabel = new QLabel("Velocidad preview"); speedLabel->setObjectName("wgSubtle");
  auto *speed = new QComboBox(); speed->addItem("0.25×", 0.25); speed->addItem("0.5×", 0.5); speed->addItem("1×", 1.0); speed->addItem("2×", 2.0); speed->setCurrentIndex(2);
  auto *halfTime = new QPushButton("½ TIEMPO"); halfTime->setToolTip("Comprime todos los puntos de esta fase: animación 2× más rápida");
  auto *doubleTime = new QPushButton("×2 TIEMPO"); doubleTime->setToolTip("Estira todos los puntos de esta fase: animación 2× más lenta");
  auto *timeReadout = new QLabel("0.00 s"); timeReadout->setObjectName("wgSubtle");
  timelineControls->addWidget(phase); timelineControls->addWidget(play); timelineControls->addWidget(stop); timelineControls->addSpacing(10);
  timelineControls->addWidget(speedLabel); timelineControls->addWidget(speed); timelineControls->addSpacing(10);
  timelineControls->addWidget(halfTime); timelineControls->addWidget(doubleTime); timelineControls->addStretch(); timelineControls->addWidget(timeReadout);
  timelineLayout->addLayout(timelineControls);
  timeline_ = new TimelineWidget(); timelineLayout->addWidget(timeline_);
  root->addWidget(timelineCard);

  auto &state = AppState::instance();
  connect(&state, &AppState::previewChanged, this, &DesignPage::refreshCanvas);
  connect(&state, &AppState::modelChanged, this, &DesignPage::rebuildLayerList);
  connect(layers_, &QListWidget::currentRowChanged, this, &DesignPage::selectLayer);
  connect(apply, &QPushButton::clicked, this, &DesignPage::applyProperties);
  connect(addText, &QPushButton::clicked, this, &DesignPage::addTextLayer);
  connect(addShape, &QPushButton::clicked, this, &DesignPage::addShapeLayer);
  connect(addImage, &QPushButton::clicked, this, &DesignPage::addImageLayer);
  connect(remove, &QPushButton::clicked, this, &DesignPage::deleteCurrentLayer);
  connect(duplicate, &QPushButton::clicked, this, &DesignPage::duplicateCurrentLayer);
  connect(up, &QPushButton::clicked, this, [this] { moveCurrentLayer(-1); });
  connect(down, &QPushButton::clicked, this, [this] { moveCurrentLayer(1); });
  connect(visible, &QPushButton::clicked, this, &DesignPage::toggleVisibility);
  connect(lock, &QPushButton::clicked, this, &DesignPage::toggleLock);
  connect(group, &QPushButton::clicked, this, &DesignPage::groupSelection);
  connect(ungroup, &QPushButton::clicked, this, &DesignPage::ungroupCurrent);
  connect(stagger, &QPushButton::clicked, this, &DesignPage::applyStagger);
  connect(importPsd, &QPushButton::clicked, this, &DesignPage::importPsdPlaceholder);
  connect(save, &QPushButton::clicked, this, &DesignPage::saveTemplate);
  connect(templates_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) { loadSelectedTemplate(); });

  connect(phase, &QComboBox::currentIndexChanged, this, [this](int index) { timeline_->setPhase(index == 0); });
  connect(play, &QPushButton::clicked, timeline_, &TimelineWidget::togglePlayback);
  connect(stop, &QPushButton::clicked, this, [this] { timeline_->stopPlayback(); timeline_->setCurrentTimeMs(0); });
  connect(speed, &QComboBox::currentIndexChanged, this, [this, speed](int index) { timeline_->setPlaybackSpeed(speed->itemData(index).toDouble()); });
  connect(halfTime, &QPushButton::clicked, this, [this] { AppState::instance().scaleTimeline(0.5, timeline_->isEntering()); timeline_->refreshCurrentFrame(); selectLayer(currentRow_); });
  connect(doubleTime, &QPushButton::clicked, this, [this] { AppState::instance().scaleTimeline(2.0, timeline_->isEntering()); timeline_->refreshCurrentFrame(); selectLayer(currentRow_); });
  connect(timeline_, &TimelineWidget::currentTimeChanged, this, [timeReadout](int ms) { timeReadout->setText(QString::number(ms / 1000.0, 'f', 2) + " s"); });
  connect(timeline_, &TimelineWidget::playbackStateChanged, this, [play](bool playing) { play->setText(playing ? "❚❚ PAUSA" : "▶ REPRODUCIR"); });
  connect(timeline_, &TimelineWidget::layerSelected, this, [this](int row) { layers_->setCurrentRow(row); });
  connect(timeline_, &TimelineWidget::timingEdited, this, [this](int row) { if (row == currentRow_) selectLayer(row); });

  refreshTemplateLibrary(); rebuildLayerList(); refreshCanvas();
  timeline_->setCurrentTimeMs(0);
}

void DesignPage::refreshTemplateLibrary()
{
  templates_->clear();
  struct Builtin { QString id; QString name; Project p; };
  const QList<Builtin> builtins = {
    {"builtin:pastor", "Pastor Clean", TemplateFactory::pastorLowerThird()},
    {"builtin:motion", "Motion Pieces", TemplateFactory::motionPiecesLowerThird()},
    {"builtin:scripture", "Versículo", TemplateFactory::scriptureLowerThird()}
  };
  for (const auto &b : builtins) {
    auto *item = new QListWidgetItem(projectIcon(b.p), b.name); item->setData(Qt::UserRole, b.id); item->setSizeHint({185, 112}); templates_->addItem(item);
  }
  for (const auto &entry : TemplateLibrary::entries()) {
    QIcon icon; if (!entry.thumbnailPath.isEmpty()) icon = QIcon(entry.thumbnailPath);
    auto *item = new QListWidgetItem(icon, entry.name); item->setData(Qt::UserRole, entry.filePath); item->setSizeHint({185, 112}); templates_->addItem(item);
  }
}

void DesignPage::loadSelectedTemplate()
{
  auto *item = templates_->currentItem(); if (!item) return;
  const QString id = item->data(Qt::UserRole).toString();
  if (id == "builtin:pastor") AppState::instance().loadProject(TemplateFactory::pastorLowerThird());
  else if (id == "builtin:motion") AppState::instance().loadMotionTemplate();
  else if (id == "builtin:scripture") AppState::instance().loadProject(TemplateFactory::scriptureLowerThird());
  else {
    Project p; QString error;
    if (TemplateLibrary::load(id, &p, &error)) AppState::instance().loadProject(p);
    else QMessageBox::warning(this, "Plantilla", error);
  }
  timeline_->setCurrentTimeMs(0);
}

void DesignPage::saveTemplate()
{
  bool ok = false; const QString name = QInputDialog::getText(this, "Guardar plantilla", "Nombre:", QLineEdit::Normal, AppState::instance().project().name, &ok);
  if (!ok || name.trimmed().isEmpty()) return;
  QString error;
  if (!TemplateLibrary::save(AppState::instance().project(), name, &error)) QMessageBox::warning(this, "Worship Graphics", error);
  else { refreshTemplateLibrary(); QMessageBox::information(this, "Worship Graphics", "Plantilla guardada con miniatura."); }
}

void DesignPage::rebuildLayerList()
{
  const QString keepId = (currentRow_ >= 0 && currentRow_ < AppState::instance().project().layers.size()) ? AppState::instance().project().layers[currentRow_].id : QString();
  layers_->blockSignals(true);
  layers_->clear();
  const auto &project = AppState::instance().project();
  int restore = -1;
  for (int i = 0; i < project.layers.size(); ++i) {
    const auto &layer = project.layers[i];
    QString type = layer.type == LayerType::Group ? "▾" : layer.type == LayerType::Text ? "T" : layer.type == LayerType::Shape ? "◆" : "▧";
    QString indent = layer.parentId.isEmpty() ? "" : "    ";
    QString flags = QString("  %1 %2").arg(layer.visible ? "●" : "○", layer.locked ? "L" : "");
    auto *item = new QListWidgetItem(indent + type + "  " + layer.name + flags); item->setData(Qt::UserRole, layer.id); layers_->addItem(item);
    if (!keepId.isEmpty() && layer.id == keepId) restore = i;
  }
  const int row = restore >= 0 ? restore : qMin(currentRow_, layers_->count() - 1);
  if (row >= 0 && row < layers_->count()) layers_->setCurrentRow(row); else currentRow_ = -1;
  layers_->blockSignals(false);
  if (row >= 0) selectLayer(row);
  timeline_->setSelectedLayer(currentRow_);
  timeline_->update();
}

void DesignPage::setCurrentRowSafe(int row)
{
  if (row >= 0 && row < layers_->count()) layers_->setCurrentRow(row); else currentRow_ = -1;
}

void DesignPage::selectLayer(int row)
{
  currentRow_ = row; timeline_->setSelectedLayer(row);
  const auto &project = AppState::instance().project();
  if (row < 0 || row >= project.layers.size()) return;
  const auto &l = project.layers[row];
  name_->setText(l.name); text_->setText(l.text); color_->setText(l.color.name(QColor::HexArgb));
  x_->setValue(qRound(l.position.x())); y_->setValue(qRound(l.position.y())); w_->setValue(qRound(l.size.width())); h_->setValue(qRound(l.size.height()));
  fontSize_->setValue(l.fontSize); opacity_->setValue(qRound(l.opacity * 100)); radius_->setValue(qRound(l.cornerRadius)); rotation_->setValue(l.rotationDeg);
  enterAnimation_->setCurrentIndex(static_cast<int>(l.enterAnimation)); exitAnimation_->setCurrentIndex(static_cast<int>(l.exitAnimation));
  enterDelay_->setValue(l.enterDelayMs); exitDelay_->setValue(l.exitDelayMs); enterDuration_->setValue(l.enterDurationMs); exitDuration_->setValue(l.exitDurationMs);
  text_->setEnabled(l.type == LayerType::Text);
}

void DesignPage::applyProperties()
{
  auto &state = AppState::instance(); auto &project = state.mutableProject();
  if (currentRow_ < 0 || currentRow_ >= project.layers.size()) return;
  auto &l = project.layers[currentRow_]; if (l.locked) { QMessageBox::information(this, "Capa bloqueada", "Desbloquea la capa antes de editarla."); return; }
  l.name = name_->text().trimmed().isEmpty() ? l.name : name_->text().trimmed();
  if (l.type == LayerType::Text) l.text = text_->text();
  const QColor c(color_->text()); if (c.isValid()) l.color = c;
  l.position = QPointF(x_->value(), y_->value()); l.size = QSizeF(w_->value(), h_->value()); l.fontSize = fontSize_->value();
  l.opacity = opacity_->value() / 100.0; l.cornerRadius = radius_->value(); l.rotationDeg = rotation_->value();
  l.enterAnimation = static_cast<AnimationPreset>(enterAnimation_->currentIndex()); l.exitAnimation = static_cast<AnimationPreset>(exitAnimation_->currentIndex());
  l.enterDelayMs = enterDelay_->value(); l.exitDelayMs = exitDelay_->value(); l.enterDurationMs = enterDuration_->value(); l.exitDurationMs = exitDuration_->value();
  state.notifyModelChanged(); timeline_->refreshCurrentFrame();
}

void DesignPage::addTextLayer()
{
  Layer l; l.id = QUuid::createUuid().toString(QUuid::WithoutBraces); l.name = "Texto nuevo"; l.type = LayerType::Text; l.position = {180, 500}; l.size = {700, 100}; l.text = "NUEVO TEXTO"; l.color = Qt::white; l.fontSize = 46;
  AppState::instance().mutableProject().layers.push_back(l); AppState::instance().notifyModelChanged(); setCurrentRowSafe(AppState::instance().project().layers.size() - 1);
}

void DesignPage::addShapeLayer()
{
  Layer l; l.id = QUuid::createUuid().toString(QUuid::WithoutBraces); l.name = "Forma nueva"; l.type = LayerType::Shape; l.position = {150, 650}; l.size = {600, 120}; l.color = QColor("#555555");
  AppState::instance().mutableProject().layers.push_back(l); AppState::instance().notifyModelChanged(); setCurrentRowSafe(AppState::instance().project().layers.size() - 1);
}

void DesignPage::addImageLayer()
{
  const QString file = QFileDialog::getOpenFileName(this, "Agregar imagen", {}, "Imágenes (*.png *.jpg *.jpeg *.webp)"); if (file.isEmpty()) return;
  Layer l; l.id = QUuid::createUuid().toString(QUuid::WithoutBraces); l.name = "Imagen"; l.type = LayerType::Image; l.position = {200, 500}; l.size = {600, 300}; l.imagePath = file; l.enterAnimation = AnimationPreset::Fade; l.exitAnimation = AnimationPreset::Fade;
  AppState::instance().mutableProject().layers.push_back(l); AppState::instance().notifyModelChanged(); setCurrentRowSafe(AppState::instance().project().layers.size() - 1);
}

void DesignPage::deleteCurrentLayer()
{
  if (currentRow_ < 0) return; const auto &layers = AppState::instance().project().layers; if (currentRow_ >= layers.size()) return;
  if (QMessageBox::question(this, "Eliminar capa", "¿Eliminar ‘" + layers[currentRow_].name + "’ y sus capas internas?") != QMessageBox::Yes) return;
  AppState::instance().removeLayer(currentRow_); currentRow_ = qMin(currentRow_, AppState::instance().project().layers.size() - 1); setCurrentRowSafe(currentRow_);
}

void DesignPage::duplicateCurrentLayer() { if (AppState::instance().duplicateLayer(currentRow_)) setCurrentRowSafe(qMin(currentRow_ + 1, layers_->count() - 1)); }
void DesignPage::moveCurrentLayer(int delta) { const int target = currentRow_ + delta; if (AppState::instance().moveLayer(currentRow_, delta)) { currentRow_ = target; setCurrentRowSafe(target); } }
void DesignPage::toggleVisibility() { AppState::instance().toggleLayerVisible(currentRow_); }
void DesignPage::toggleLock() { AppState::instance().toggleLayerLocked(currentRow_); }

void DesignPage::groupSelection()
{
  QVector<int> rows; for (auto *item : layers_->selectedItems()) rows << layers_->row(item);
  if (!AppState::instance().groupLayers(rows)) QMessageBox::information(this, "Agrupar", "Selecciona dos o más capas.");
}
void DesignPage::ungroupCurrent() { AppState::instance().ungroupLayer(currentRow_); }
void DesignPage::applyStagger() { AppState::instance().staggerLayers(80); timeline_->refreshCurrentFrame(); selectLayer(currentRow_); }

void DesignPage::importPsdPlaceholder()
{
  const QString file = QFileDialog::getOpenFileName(this, "Importar diseño de Photoshop", {}, "Photoshop (*.psd *.psb)"); if (file.isEmpty()) return;
  QMessageBox::information(this, "Importador PSD", "PSD seleccionado. El parser de capas PSD/PSB sigue siendo el siguiente módulo del motor; esta versión ya tiene grupos, imágenes, biblioteca y un Timeline editable para recibir esas capas sin rehacer el editor.");
}

void DesignPage::refreshCanvas()
{
  const QImage frame = AppState::instance().previewFrame();
  canvas_->setPixmap(QPixmap::fromImage(frame).scaled(canvas_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

} // namespace wg
