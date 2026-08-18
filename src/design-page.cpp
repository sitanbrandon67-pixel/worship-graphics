#include "design-page.hpp"
#include "design-canvas-widget.hpp"
#include "app-state.hpp"
#include "graphics-renderer.hpp"
#include "template-factory.hpp"
#include "template-library.hpp"
#include "theme.hpp"
#include "timeline-widget.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QPixmap>
#include <QScrollArea>
#include <QSpinBox>
#include <QStyle>
#include <QSplitter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QVector>
#include <QUuid>
#include <algorithm>

namespace wg {


namespace {
class ResponsiveGrid final : public QWidget {
public:
  explicit ResponsiveGrid(int preferredCellWidth, QWidget *parent = nullptr)
      : QWidget(parent), preferredCellWidth_(qMax(90, preferredCellWidth))
  {
    layout_ = new QGridLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setHorizontalSpacing(8);
    layout_->setVerticalSpacing(7);
  }

  void addItem(QWidget *widget)
  {
    if (!widget) return;
    items_.push_back(widget);
    relayout();
  }

protected:
  void resizeEvent(QResizeEvent *event) override
  {
    QWidget::resizeEvent(event);
    relayout();
  }

private:
  void relayout()
  {
    if (items_.isEmpty()) return;
    const int available = qMax(1, width());
    const int columns = qBound(1, available / preferredCellWidth_, items_.size());

    for (QWidget *widget : items_)
      layout_->removeWidget(widget);

    for (int i = 0; i < items_.size(); ++i)
      layout_->addWidget(items_[i], i / columns, i % columns);

    for (int c = 0; c < items_.size(); ++c)
      layout_->setColumnStretch(c, c < columns ? 1 : 0);
  }

  QGridLayout *layout_ = nullptr;
  QVector<QWidget *> items_;
  int preferredCellWidth_ = 140;
};
} // namespace

static QStringList animationNames()
{
  return {"Ninguna", "Fade", "Desde izquierda", "Desde derecha", "Desde arriba", "Desde abajo", "Zoom", "Pop", "Expandir horizontal", "Expandir vertical"};
}

static QIcon projectIcon(const Project &project)
{
  const QImage img = GraphicsRenderer::render(project).scaled(180, 102, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  return QIcon(QPixmap::fromImage(img));
}


static QPushButton *iconButton(QWidget *owner, QStyle::StandardPixmap icon, const QString &tip)
{
  auto *button = new QPushButton();
  button->setObjectName("wgIconButton");
  button->setIcon(owner->style()->standardIcon(icon));
  button->setIconSize({15, 15});
  button->setToolTip(tip);
  return button;
}

static QPushButton *glyphButton(const QString &glyph, const QString &tip)
{
  auto *button = new QPushButton(glyph);
  button->setObjectName("wgIconButton");
  button->setToolTip(tip);
  return button;
}

DesignPage::DesignPage(QWidget *parent) : QWidget(parent)
{
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(10, 8, 10, 8);
  root->setSpacing(7);

  auto *libraryCard = new QFrame();
  libraryCard->setObjectName("wgCard");
  applySoftShadow(libraryCard);
  auto *libraryLayout = new QVBoxLayout(libraryCard);
  libraryLayout->setContentsMargins(9, 8, 9, 8);
  libraryLayout->setSpacing(5);

  auto *libraryTitle = new QHBoxLayout();
  auto *title = new QLabel("PLANTILLAS");
  title->setObjectName("wgSectionTitle");
  auto *selectedTemplateLabel = new QLabel("Selecciona una plantilla");
  selectedTemplateLabel->setObjectName("wgSubtle");
  auto *bibleDefaultLabel = new QLabel();
  bibleDefaultLabel->setObjectName("wgSubtle");

  auto *save = iconButton(this, QStyle::SP_DialogSaveButton, "Guardar el diseño actual como plantilla");
  auto *openTemplate = iconButton(this, QStyle::SP_DialogOpenButton, "Abrir la plantilla seleccionada en Diseño");
  auto *bibleDefaultButton = glyphButton("★", "Usar esta plantilla como la plantilla bíblica predeterminada");
  auto *deleteTemplate = iconButton(this, QStyle::SP_TrashIcon, "Eliminar plantilla personal u ocultar una integrada");
  deleteTemplate->setObjectName("wgIconButton");
  auto *restoreBuiltins = iconButton(this, QStyle::SP_BrowserReload, "Restaurar plantillas integradas ocultas");

  libraryTitle->addWidget(title);
  libraryTitle->addSpacing(8);
  libraryTitle->addWidget(selectedTemplateLabel, 1);
  libraryTitle->addWidget(bibleDefaultLabel);
  libraryTitle->addWidget(openTemplate);
  libraryTitle->addWidget(bibleDefaultButton);
  libraryTitle->addWidget(save);
  libraryTitle->addWidget(deleteTemplate);
  libraryTitle->addWidget(restoreBuiltins);
  libraryLayout->addLayout(libraryTitle);

  templates_ = new QListWidget();
  templates_->setObjectName("wgTemplateLibrary");
  templates_->setSelectionMode(QAbstractItemView::SingleSelection);
  templates_->setSelectionBehavior(QAbstractItemView::SelectItems);
  templates_->setFocusPolicy(Qt::StrongFocus);
  templates_->setViewMode(QListView::IconMode);
  templates_->setResizeMode(QListView::Adjust);
  templates_->setMovement(QListView::Static);
  templates_->setWrapping(false);
  templates_->setFlow(QListView::LeftToRight);
  templates_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  templates_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  templates_->setIconSize({132, 74});
  templates_->setSpacing(6);
  templates_->setFixedHeight(96);
  libraryLayout->addWidget(templates_);
  root->addWidget(libraryCard);

  auto *toolbar = new QHBoxLayout();
  toolbar->setSpacing(4);
  auto *importPsd = glyphButton("Ps", "Importar PSD/PSB");
  auto *addText = glyphButton("T", "Agregar capa de texto");
  auto *addShape = glyphButton("▭", "Agregar forma");
  auto *addImage = iconButton(this, QStyle::SP_FileIcon, "Agregar imagen");
  auto *duplicate = glyphButton("⧉", "Duplicar capa");
  auto *remove = iconButton(this, QStyle::SP_TrashIcon, "Eliminar capa");
  remove->setObjectName("wgIconButton");
  auto *stagger = glyphButton("⇥", "Escalonar animaciones 80 ms");
  toolbar->addWidget(importPsd);
  toolbar->addWidget(addText);
  toolbar->addWidget(addShape);
  toolbar->addWidget(addImage);
  toolbar->addWidget(duplicate);
  toolbar->addWidget(remove);
  toolbar->addWidget(stagger);
  toolbar->addStretch();
  root->addLayout(toolbar);

  auto *splitter = new QSplitter(Qt::Horizontal);
  splitter->setChildrenCollapsible(false);
  splitter->setHandleWidth(8);

  auto *layersCard = new QFrame(); layersCard->setObjectName("wgCard"); layersCard->setMinimumWidth(190); layersCard->setMaximumWidth(320); applySoftShadow(layersCard);
  layersCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  auto *layersLayout = new QVBoxLayout(layersCard);
  auto *layersTitle = new QLabel("CAPAS"); layersTitle->setObjectName("wgSectionTitle"); layersLayout->addWidget(layersTitle);
  layers_ = new QListWidget();
  layers_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  layersLayout->addWidget(layers_, 1);
  auto *layerActions1 = new QHBoxLayout();
  layerActions1->setSpacing(4);
  auto *up = iconButton(this, QStyle::SP_ArrowUp, "Subir capa");
  auto *down = iconButton(this, QStyle::SP_ArrowDown, "Bajar capa");
  auto *visible = glyphButton("◉", "Mostrar / ocultar capa");
  auto *lock = glyphButton("L", "Bloquear / desbloquear capa");
  auto *group = glyphButton("G", "Agrupar capas seleccionadas");
  auto *ungroup = glyphButton("U", "Desagrupar");
  layerActions1->addWidget(up); layerActions1->addWidget(down); layerActions1->addWidget(visible); layerActions1->addWidget(lock); layerActions1->addWidget(group); layerActions1->addWidget(ungroup);
  layerActions1->addStretch();
  layersLayout->addLayout(layerActions1);

  auto *canvasCard = new QFrame(); canvasCard->setObjectName("wgCard"); applySoftShadow(canvasCard);
  canvasCard->setMinimumWidth(320);
  canvasCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto *canvasLayout = new QVBoxLayout(canvasCard);
  auto *canvasHeader = new QHBoxLayout();
  auto *canvasTitle = new QLabel("CANVAS"); canvasTitle->setObjectName("wgSectionTitle");
  canvasInfo_ = new QLabel(); canvasInfo_->setObjectName("wgSubtle");
  canvasHeader->addWidget(canvasTitle); canvasHeader->addStretch(); canvasHeader->addWidget(canvasInfo_); canvasLayout->addLayout(canvasHeader);
  canvas_ = new DesignCanvasWidget();
  canvasLayout->addWidget(canvas_, 1);

  auto *propertiesCard = new QFrame(); propertiesCard->setObjectName("wgCard"); applySoftShadow(propertiesCard);
  propertiesCard->setMinimumWidth(250);
  propertiesCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  auto *propertiesLayout = new QVBoxLayout(propertiesCard);
  auto *propTitle = new QLabel("PROPIEDADES"); propTitle->setObjectName("wgSectionTitle"); propertiesLayout->addWidget(propTitle);

  bibleTemplate_ = new QCheckBox("PLANTILLA BÍBLICA");
  bibleTemplate_->setToolTip("Destina esta plantilla a {{VERSICULO}} y {{REFERENCIA}}");
  propertiesLayout->addWidget(bibleTemplate_);

  auto *form = new QFormLayout();
  name_ = new QLineEdit(); text_ = new QLineEdit(); color_ = new QLineEdit();
  x_ = new QSpinBox(); y_ = new QSpinBox(); w_ = new QSpinBox(); h_ = new QSpinBox(); fontSize_ = new QSpinBox();
  minFontSize_ = new QSpinBox(); maxLines_ = new QSpinBox();
  opacity_ = new QSpinBox(); radius_ = new QSpinBox(); rotation_ = new QDoubleSpinBox();
  enterDelay_ = new QSpinBox(); exitDelay_ = new QSpinBox(); enterDuration_ = new QSpinBox(); exitDuration_ = new QSpinBox();
  for (auto *spin : {x_, y_, w_, h_}) spin->setRange(-8000, 8000);
  fontSize_->setRange(8, 400); minFontSize_->setRange(6, 400); maxLines_->setRange(1, 8);
  opacity_->setRange(0, 100); radius_->setRange(0, 300); rotation_->setRange(-360, 360); rotation_->setDecimals(1);
  enterDelay_->setRange(0, 30000); exitDelay_->setRange(0, 30000); enterDuration_->setRange(80, 30000); exitDuration_->setRange(80, 30000);
  enterAnimation_ = new QComboBox(); exitAnimation_ = new QComboBox(); enterAnimation_->addItems(animationNames()); exitAnimation_->addItems(animationNames());
  textAlignH_ = new QComboBox(); textAlignH_->addItems({"Izquierda", "Centro", "Derecha"});
  textAlignV_ = new QComboBox(); textAlignV_->addItems({"Arriba", "Centro", "Abajo"});
  autoFit_ = new QCheckBox("Autoajustar fuente al marco");
  wrap_ = new QCheckBox("Ajustar líneas dentro del marco");
  splitOverflow_ = new QCheckBox("Dividir si llega al límite");

  form->addRow("Nombre", name_); form->addRow("Texto", text_); form->addRow("Color", color_);
  form->addRow("X", x_); form->addRow("Y", y_); form->addRow("Ancho", w_); form->addRow("Alto", h_);
  form->addRow("Fuente base", fontSize_); form->addRow("Fuente mínima", minFontSize_); form->addRow("Máx. líneas", maxLines_);
  form->addRow("Texto horizontal", textAlignH_); form->addRow("Texto vertical", textAlignV_);
  form->addRow("Opacidad %", opacity_); form->addRow("Radio", radius_); form->addRow("Rotación", rotation_);
  form->addRow("Entrada", enterAnimation_); form->addRow("Delay entrada", enterDelay_); form->addRow("Duración entrada", enterDuration_);
  form->addRow("Salida", exitAnimation_); form->addRow("Delay salida", exitDelay_); form->addRow("Duración salida", exitDuration_);
  propertiesLayout->addLayout(form);
  propertiesLayout->addWidget(autoFit_); propertiesLayout->addWidget(wrap_); propertiesLayout->addWidget(splitOverflow_);

  auto *fieldTitle = new QLabel("CAMPOS BÍBLICOS"); fieldTitle->setObjectName("wgSectionTitle"); propertiesLayout->addWidget(fieldTitle);
  auto *fieldRow = new QHBoxLayout();
  auto *verseField = new QPushButton("{{VERSICULO}}");
  auto *referenceField = new QPushButton("{{REFERENCIA}}");
  fieldRow->addWidget(verseField); fieldRow->addWidget(referenceField); propertiesLayout->addLayout(fieldRow);

  auto *alignTitle = new QLabel("ALINEAR CAPA EN CANVAS"); alignTitle->setObjectName("wgSectionTitle"); propertiesLayout->addWidget(alignTitle);
  auto *alignRow1 = new QHBoxLayout();
  alignRow1->setSpacing(4);
  auto *alignLeft = glyphButton("←|", "Alinear capa a la izquierda");
  auto *alignCenterH = glyphButton("↔", "Centrar capa horizontalmente");
  auto *alignRight = glyphButton("|→", "Alinear capa a la derecha");
  auto *alignTop = glyphButton("↑", "Alinear capa arriba");
  auto *alignCenterV = glyphButton("↕", "Centrar capa verticalmente");
  auto *alignBottom = glyphButton("↓", "Alinear capa abajo");
  alignRow1->addWidget(alignLeft); alignRow1->addWidget(alignCenterH); alignRow1->addWidget(alignRight);
  alignRow1->addWidget(alignTop); alignRow1->addWidget(alignCenterV); alignRow1->addWidget(alignBottom); alignRow1->addStretch();
  propertiesLayout->addLayout(alignRow1);

  auto *apply = new QPushButton("APLICAR CAMBIOS"); apply->setObjectName("wgPrimary"); propertiesLayout->addWidget(apply); propertiesLayout->addStretch();

  auto *propertiesScroll = new QScrollArea();
  propertiesScroll->setFrameShape(QFrame::NoFrame);
  propertiesScroll->setWidgetResizable(true);
  propertiesScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  propertiesScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  propertiesScroll->setMinimumWidth(260);
  propertiesScroll->setWidget(propertiesCard);

  splitter->addWidget(layersCard);
  splitter->addWidget(canvasCard);
  splitter->addWidget(propertiesScroll);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  splitter->setStretchFactor(2, 0);
  splitter->setSizes({215, 860, 285});

  root->addWidget(splitter, 1);

  auto *timelineCard = new QFrame(); timelineCard->setObjectName("wgCard"); applySoftShadow(timelineCard);
  auto *timelineLayout = new QVBoxLayout(timelineCard); timelineLayout->setContentsMargins(10, 10, 10, 10); timelineLayout->setSpacing(8);
  auto *timelineControls = new ResponsiveGrid(145);
  auto *phase = new QComboBox(); phase->addItems({"ENTRADA", "SALIDA"});
  auto *play = new QPushButton("▶ REPRODUCIR"); play->setObjectName("wgPrimary");
  auto *stop = new QPushButton("■ DETENER");
  auto *speedLabel = new QLabel("Velocidad preview"); speedLabel->setObjectName("wgSubtle");
  auto *speed = new QComboBox(); speed->addItem("0.25×", 0.25); speed->addItem("0.5×", 0.5); speed->addItem("1×", 1.0); speed->addItem("2×", 2.0); speed->setCurrentIndex(2);
  auto *halfTime = new QPushButton("½ TIEMPO"); halfTime->setToolTip("Comprime todos los puntos de esta fase: animación 2× más rápida");
  auto *doubleTime = new QPushButton("×2 TIEMPO"); doubleTime->setToolTip("Estira todos los puntos de esta fase: animación 2× más lenta");
  auto *timeReadout = new QLabel("0.00 s"); timeReadout->setObjectName("wgSubtle");
  timelineControls->addItem(phase);
  timelineControls->addItem(play);
  timelineControls->addItem(stop);
  timelineControls->addItem(speedLabel);
  timelineControls->addItem(speed);
  timelineControls->addItem(halfTime);
  timelineControls->addItem(doubleTime);
  timelineControls->addItem(timeReadout);
  timelineLayout->addWidget(timelineControls);

  timeline_ = new TimelineWidget();
  timeline_->setMinimumWidth(680);
  auto *timelineScroll = new QScrollArea();
  timelineScroll->setObjectName("wgTimelineScroll");
  timelineScroll->setFrameShape(QFrame::NoFrame);
  timelineScroll->setWidgetResizable(true);
  timelineScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  timelineScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  timelineScroll->setWidget(timeline_);
  timelineLayout->addWidget(timelineScroll);
  timelineCard->setMinimumHeight(215);
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

  auto refreshBibleDefaultLabel = [bibleDefaultLabel]() {
    const QString id = TemplateLibrary::preferredBibleTemplate();
    const QString name = TemplateLibrary::displayNameForId(id);
    bibleDefaultLabel->setText(name.isEmpty() ? QString("Biblia: sin plantilla") : QString("Biblia: ★ %1").arg(name));
  };

  auto refreshSelectedTemplateLabel = [this, selectedTemplateLabel, bibleDefaultButton, deleteTemplate, openTemplate]() {
    auto *item = templates_->currentItem();
    if (!item) {
      selectedTemplateLabel->setText("Selecciona una plantilla");
      bibleDefaultButton->setEnabled(false);
      deleteTemplate->setEnabled(false);
      openTemplate->setEnabled(false);
      return;
    }

    const QString id = item->data(Qt::UserRole).toString();
    selectedTemplateLabel->setText("Seleccionada: " + TemplateLibrary::displayNameForId(id));
    bibleDefaultButton->setEnabled(TemplateLibrary::isBibleTemplateId(id));
    deleteTemplate->setEnabled(true);
    openTemplate->setEnabled(true);
  };

  connect(templates_, &QListWidget::itemClicked, this, [this, refreshSelectedTemplateLabel](QListWidgetItem *item) {
    if (!item) return;
    templates_->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
    refreshSelectedTemplateLabel();
  });
  connect(templates_, &QListWidget::currentItemChanged, this, [refreshSelectedTemplateLabel](QListWidgetItem *, QListWidgetItem *) {
    refreshSelectedTemplateLabel();
  });

  connect(openTemplate, &QPushButton::clicked, this, [this] {
    if (templates_->currentItem()) loadSelectedTemplate();
  });

  connect(bibleDefaultButton, &QPushButton::clicked, this, [this, refreshBibleDefaultLabel] {
    auto *item = templates_->currentItem();
    if (!item) return;
    const QString id = item->data(Qt::UserRole).toString();
    if (!TemplateLibrary::isBibleTemplateId(id)) {
      QMessageBox::information(this, "Plantilla bíblica", "Esta plantilla no contiene {{VERSICULO}} y {{REFERENCIA}}.");
      return;
    }
    TemplateLibrary::setBibleDefaultTemplate(id);
    refreshBibleDefaultLabel();
  });

  connect(deleteTemplate, &QPushButton::clicked, this, [this, refreshBibleDefaultLabel, refreshSelectedTemplateLabel] {
    auto *item = templates_->currentItem();
    if (!item) return;
    const QString id = item->data(Qt::UserRole).toString();
    const bool builtin = id.startsWith("builtin:");
    const QString verb = builtin ? "ocultar" : "eliminar";
    if (QMessageBox::question(this, "Plantillas", "¿Quieres " + verb + " ‘" + TemplateLibrary::displayNameForId(id) + "’?") != QMessageBox::Yes)
      return;
    QString error;
    if (!TemplateLibrary::removeOrHide(id, &error)) {
      QMessageBox::warning(this, "Plantillas", error);
      return;
    }
    refreshTemplateLibrary();
    refreshBibleDefaultLabel();
    refreshSelectedTemplateLabel();
  });

  connect(restoreBuiltins, &QPushButton::clicked, this, [this, refreshBibleDefaultLabel, refreshSelectedTemplateLabel] {
    TemplateLibrary::restoreBuiltins();
    refreshTemplateLibrary();
    refreshBibleDefaultLabel();
    refreshSelectedTemplateLabel();
  });

  refreshBibleDefaultLabel();
  refreshSelectedTemplateLabel();

  connect(canvas_, &DesignCanvasWidget::layerSelected, this, [this](int row) {
    if (row >= 0) layers_->setCurrentRow(row);
    else layers_->clearSelection();
  });
  connect(canvas_, &DesignCanvasWidget::layerGeometryPreviewChanged, this, [this](int row) {
    currentRow_ = row;
    selectLayer(row);
    timeline_->refreshCurrentFrame();
  });
  connect(canvas_, &DesignCanvasWidget::layerGeometryCommitted, this, [this](int row) {
    currentRow_ = row;
    selectLayer(row);
    timeline_->refreshCurrentFrame();
  });

  connect(canvas_, &DesignCanvasWidget::viewScaleChanged, this, [this](qreal) { refreshCanvas(); });

  connect(bibleTemplate_, &QCheckBox::toggled, this, &DesignPage::markBibleTemplate);
  connect(verseField, &QPushButton::clicked, this, &DesignPage::markAsVerseField);
  connect(referenceField, &QPushButton::clicked, this, &DesignPage::markAsReferenceField);
  connect(alignLeft, &QPushButton::clicked, this, &DesignPage::alignLayerLeft);
  connect(alignCenterH, &QPushButton::clicked, this, &DesignPage::alignLayerCenterH);
  connect(alignRight, &QPushButton::clicked, this, &DesignPage::alignLayerRight);
  connect(alignTop, &QPushButton::clicked, this, &DesignPage::alignLayerTop);
  connect(alignCenterV, &QPushButton::clicked, this, &DesignPage::alignLayerCenterV);
  connect(alignBottom, &QPushButton::clicked, this, &DesignPage::alignLayerBottom);

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
  const QString bibleDefault = TemplateLibrary::preferredBibleTemplate();
  templates_->clear();

  struct Builtin { QString id; QString name; Project p; };
  const QList<Builtin> builtins = {
    {"builtin:pastor", "Pastor Clean", TemplateFactory::pastorLowerThird()},
    {"builtin:motion", "Motion Pieces", TemplateFactory::motionPiecesLowerThird()},
    {"builtin:scripture", "Versículo", TemplateFactory::scriptureLowerThird()},
    {"builtin:sermon", "Tema", TemplateFactory::sermonTitleLowerThird()},
    {"builtin:worship", "Alabanza", TemplateFactory::worshipLowerThird()},
    {"builtin:announcement", "Anuncio", TemplateFactory::announcementLowerThird()}
  };

  for (const auto &b : builtins) {
    if (TemplateLibrary::isBuiltinHidden(b.id)) continue;
    QString label = b.name;
    if (b.id == bibleDefault) label.prepend("★ ");
    auto *item = new QListWidgetItem(projectIcon(b.p), label);
    item->setData(Qt::UserRole, b.id);
    item->setToolTip("Integrada · clic para seleccionar · doble clic para abrir");
    item->setSizeHint({152, 94});
    templates_->addItem(item);
  }

  for (const auto &entry : TemplateLibrary::entries()) {
    QIcon icon;
    if (!entry.thumbnailPath.isEmpty()) icon = QIcon(entry.thumbnailPath);
    QString label = entry.name;
    if (entry.filePath == bibleDefault) label.prepend("★ ");
    auto *item = new QListWidgetItem(icon, label);
    item->setData(Qt::UserRole, entry.filePath);
    item->setToolTip(entry.usage == TemplateUsage::BibleText
                         ? "Mía · plantilla bíblica · clic para seleccionar"
                         : "Mía · clic para seleccionar");
    item->setSizeHint({152, 94});
    templates_->addItem(item);
  }
}

void DesignPage::loadSelectedTemplate()
{
  auto *item = templates_->currentItem(); if (!item) return;
  const QString id = item->data(Qt::UserRole).toString();
  if (id == "builtin:pastor") AppState::instance().loadProject(TemplateFactory::pastorLowerThird());
  else if (id == "builtin:motion") AppState::instance().loadMotionTemplate();
  else if (id == "builtin:scripture") AppState::instance().loadProject(TemplateFactory::scriptureLowerThird());
  else if (id == "builtin:sermon") AppState::instance().loadProject(TemplateFactory::sermonTitleLowerThird());
  else if (id == "builtin:worship") AppState::instance().loadProject(TemplateFactory::worshipLowerThird());
  else if (id == "builtin:announcement") AppState::instance().loadProject(TemplateFactory::announcementLowerThird());
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
  currentRow_ = row;
  timeline_->setSelectedLayer(row);
  canvas_->setSelectedLayer(row);
  const auto &project = AppState::instance().project();
  if (row < 0 || row >= project.layers.size()) return;
  const auto &l = project.layers[row];
  name_->setText(l.name); text_->setText(l.text); color_->setText(l.color.name(QColor::HexArgb));
  x_->setValue(qRound(l.position.x())); y_->setValue(qRound(l.position.y())); w_->setValue(qRound(l.size.width())); h_->setValue(qRound(l.size.height()));
  fontSize_->setValue(l.fontSize); minFontSize_->setValue(l.minFontSize); maxLines_->setValue(l.maxLines);
  opacity_->setValue(qRound(l.opacity * 100)); radius_->setValue(qRound(l.cornerRadius)); rotation_->setValue(l.rotationDeg);
  enterAnimation_->setCurrentIndex(static_cast<int>(l.enterAnimation)); exitAnimation_->setCurrentIndex(static_cast<int>(l.exitAnimation));
  enterDelay_->setValue(l.enterDelayMs); exitDelay_->setValue(l.exitDelayMs); enterDuration_->setValue(l.enterDurationMs); exitDuration_->setValue(l.exitDurationMs);
  textAlignH_->setCurrentIndex(static_cast<int>(l.textHorizontalAlign));
  textAlignV_->setCurrentIndex(static_cast<int>(l.textVerticalAlign));
  autoFit_->setChecked(l.textAutoFit); wrap_->setChecked(l.textWrap); splitOverflow_->setChecked(l.splitOverflow);
  bibleTemplate_->blockSignals(true); bibleTemplate_->setChecked(project.usage == TemplateUsage::BibleText); bibleTemplate_->blockSignals(false);
  const bool textLayer = l.type == LayerType::Text;
  text_->setEnabled(textLayer); fontSize_->setEnabled(textLayer); minFontSize_->setEnabled(textLayer); maxLines_->setEnabled(textLayer);
  textAlignH_->setEnabled(textLayer); textAlignV_->setEnabled(textLayer); autoFit_->setEnabled(textLayer); wrap_->setEnabled(textLayer); splitOverflow_->setEnabled(textLayer);
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
  l.minFontSize = qMin(l.fontSize, minFontSize_->value()); l.maxLines = maxLines_->value();
  l.textHorizontalAlign = static_cast<TextHorizontalAlign>(textAlignH_->currentIndex());
  l.textVerticalAlign = static_cast<TextVerticalAlign>(textAlignV_->currentIndex());
  l.textAutoFit = autoFit_->isChecked(); l.textWrap = wrap_->isChecked(); l.splitOverflow = splitOverflow_->isChecked();
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

bool DesignPage::currentLayerIsText() const
{
  const auto &layers = AppState::instance().project().layers;
  return currentRow_ >= 0 && currentRow_ < layers.size() && layers[currentRow_].type == LayerType::Text;
}

void DesignPage::markBibleTemplate(bool checked)
{
  auto &state = AppState::instance();
  state.mutableProject().usage = checked ? TemplateUsage::BibleText : TemplateUsage::Generic;
  state.notifyModelChanged();
}

void DesignPage::markAsVerseField()
{
  if (!currentLayerIsText()) return;
  auto &state = AppState::instance();
  state.mutableProject().layers[currentRow_].name = "{{VERSICULO}}";
  state.mutableProject().usage = TemplateUsage::BibleText;
  state.notifyModelChanged();
  selectLayer(currentRow_);
}

void DesignPage::markAsReferenceField()
{
  if (!currentLayerIsText()) return;
  auto &state = AppState::instance();
  state.mutableProject().layers[currentRow_].name = "{{REFERENCIA}}";
  state.mutableProject().usage = TemplateUsage::BibleText;
  state.notifyModelChanged();
  selectLayer(currentRow_);
}

void DesignPage::alignLayerLeft()
{
  auto &state = AppState::instance(); auto &p = state.mutableProject();
  if (currentRow_ < 0 || currentRow_ >= p.layers.size()) return;
  p.layers[currentRow_].position.setX(0.0); state.notifyModelChanged(); selectLayer(currentRow_); timeline_->refreshCurrentFrame();
}

void DesignPage::alignLayerCenterH()
{
  auto &state = AppState::instance(); auto &p = state.mutableProject();
  if (currentRow_ < 0 || currentRow_ >= p.layers.size()) return;
  p.layers[currentRow_].position.setX((p.canvas.width() - p.layers[currentRow_].size.width()) / 2.0); state.notifyModelChanged(); selectLayer(currentRow_); timeline_->refreshCurrentFrame();
}

void DesignPage::alignLayerRight()
{
  auto &state = AppState::instance(); auto &p = state.mutableProject();
  if (currentRow_ < 0 || currentRow_ >= p.layers.size()) return;
  p.layers[currentRow_].position.setX(p.canvas.width() - p.layers[currentRow_].size.width()); state.notifyModelChanged(); selectLayer(currentRow_); timeline_->refreshCurrentFrame();
}

void DesignPage::alignLayerTop()
{
  auto &state = AppState::instance(); auto &p = state.mutableProject();
  if (currentRow_ < 0 || currentRow_ >= p.layers.size()) return;
  p.layers[currentRow_].position.setY(0.0); state.notifyModelChanged(); selectLayer(currentRow_); timeline_->refreshCurrentFrame();
}

void DesignPage::alignLayerCenterV()
{
  auto &state = AppState::instance(); auto &p = state.mutableProject();
  if (currentRow_ < 0 || currentRow_ >= p.layers.size()) return;
  p.layers[currentRow_].position.setY((p.canvas.height() - p.layers[currentRow_].size.height()) / 2.0); state.notifyModelChanged(); selectLayer(currentRow_); timeline_->refreshCurrentFrame();
}

void DesignPage::alignLayerBottom()
{
  auto &state = AppState::instance(); auto &p = state.mutableProject();
  if (currentRow_ < 0 || currentRow_ >= p.layers.size()) return;
  p.layers[currentRow_].position.setY(p.canvas.height() - p.layers[currentRow_].size.height()); state.notifyModelChanged(); selectLayer(currentRow_); timeline_->refreshCurrentFrame();
}

void DesignPage::refreshCanvas()
{
  canvas_->update();
  const QSize size = AppState::instance().project().canvas;
  if (size.width() <= 0 || size.height() <= 0) {
    canvasInfo_->clear();
    return;
  }

  const qreal ratio = qreal(size.width()) / qreal(size.height());
  const QString aspect = qAbs(ratio - (16.0 / 9.0)) < 0.01 ? QString("16:9") : QString::number(ratio, 'f', 2);
  canvasInfo_->setText(QString("%1×%2 · %3 · vista %4%")
                           .arg(size.width())
                           .arg(size.height())
                           .arg(aspect)
                           .arg(qRound(canvas_->viewScale() * 100.0)));
}


} // namespace wg
