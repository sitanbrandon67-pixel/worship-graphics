#include "design-page.hpp"
#include "app-state.hpp"

#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QUuid>

namespace wg {

DesignPage::DesignPage(QWidget *parent) : QWidget(parent)
{
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(18, 18, 18, 18);
  root->setSpacing(12);

  auto *toolbar = new QHBoxLayout();
  auto *importPsd = new QPushButton("IMPORTAR PSD");
  auto *addText = new QPushButton("+ TEXTO");
  auto *addShape = new QPushButton("+ FORMA");
  auto *saveTemplate = new QPushButton("GUARDAR PLANTILLA");
  toolbar->addWidget(importPsd);
  toolbar->addWidget(addText);
  toolbar->addWidget(addShape);
  toolbar->addStretch();
  toolbar->addWidget(saveTemplate);
  root->addLayout(toolbar);

  auto *body = new QHBoxLayout();
  body->setSpacing(12);

  auto *layersCard = new QFrame();
  layersCard->setObjectName("wgCard");
  layersCard->setFixedWidth(250);
  auto *layersLayout = new QVBoxLayout(layersCard);
  layersLayout->addWidget(new QLabel("CAPAS"));
  layers_ = new QListWidget();
  layers_->setDragDropMode(QAbstractItemView::InternalMove);
  layersLayout->addWidget(layers_, 1);
  body->addWidget(layersCard);

  auto *canvasCard = new QFrame();
  canvasCard->setObjectName("wgCard");
  auto *canvasLayout = new QVBoxLayout(canvasCard);
  canvasLayout->addWidget(new QLabel("CANVAS · AUTO-ADAPTABLE"));
  canvas_ = new QLabel();
  canvas_->setObjectName("wgScreen");
  canvas_->setAlignment(Qt::AlignCenter);
  canvas_->setMinimumSize(480, 270);
  canvasLayout->addWidget(canvas_, 1);
  body->addWidget(canvasCard, 1);

  auto *propertiesCard = new QFrame();
  propertiesCard->setObjectName("wgCard");
  propertiesCard->setFixedWidth(280);
  auto *propertiesLayout = new QVBoxLayout(propertiesCard);
  propertiesLayout->addWidget(new QLabel("PROPIEDADES"));
  auto *form = new QFormLayout();
  text_ = new QLineEdit();
  x_ = new QSpinBox(); y_ = new QSpinBox(); w_ = new QSpinBox(); h_ = new QSpinBox(); fontSize_ = new QSpinBox();
  for (auto *spin : {x_, y_, w_, h_}) { spin->setRange(-8000, 8000); }
  fontSize_->setRange(8, 400);
  enterAnimation_ = new QComboBox();
  enterAnimation_->addItems({"Ninguna", "Fade", "Desde izquierda", "Desde derecha", "Desde arriba", "Desde abajo", "Zoom"});
  form->addRow("Texto", text_);
  form->addRow("X", x_);
  form->addRow("Y", y_);
  form->addRow("Ancho", w_);
  form->addRow("Alto", h_);
  form->addRow("Fuente", fontSize_);
  form->addRow("Entrada", enterAnimation_);
  propertiesLayout->addLayout(form);
  auto *apply = new QPushButton("APLICAR CAMBIOS");
  apply->setObjectName("wgPrimary");
  propertiesLayout->addWidget(apply);
  propertiesLayout->addStretch();
  body->addWidget(propertiesCard);
  root->addLayout(body, 1);

  auto &state = AppState::instance();
  connect(&state, &AppState::previewChanged, this, &DesignPage::refreshCanvas);
  connect(&state, &AppState::modelChanged, this, &DesignPage::rebuildLayerList);
  connect(layers_, &QListWidget::currentRowChanged, this, &DesignPage::selectLayer);
  connect(apply, &QPushButton::clicked, this, &DesignPage::applyProperties);
  connect(addText, &QPushButton::clicked, this, &DesignPage::addTextLayer);
  connect(addShape, &QPushButton::clicked, this, &DesignPage::addShapeLayer);
  connect(importPsd, &QPushButton::clicked, this, &DesignPage::importPsdPlaceholder);
  connect(saveTemplate, &QPushButton::clicked, this, [this] {
    QMessageBox::information(this, "Worship Graphics", "La biblioteca de plantillas ya está separada en la arquitectura. La persistencia .wgtpl se implementa en la siguiente iteración.");
  });

  rebuildLayerList();
  refreshCanvas();
}

void DesignPage::rebuildLayerList()
{
  const int keep = layers_->currentRow();
  layers_->clear();
  const auto &project = AppState::instance().project();
  for (const auto &layer : project.layers) {
    QString prefix;
    if (layer.type == LayerType::Group) prefix = "▾  ";
    else if (layer.type == LayerType::Text) prefix = "T   ";
    else if (layer.type == LayerType::Shape) prefix = "◆   ";
    else prefix = "▧   ";
    layers_->addItem(prefix + layer.name + (layer.locked ? "  🔒" : ""));
  }
  if (keep >= 0 && keep < layers_->count()) layers_->setCurrentRow(keep);
}

void DesignPage::selectLayer(int row)
{
  currentRow_ = row;
  const auto &project = AppState::instance().project();
  if (row < 0 || row >= project.layers.size()) return;
  const auto &layer = project.layers[row];
  text_->setText(layer.text);
  x_->setValue(qRound(layer.position.x()));
  y_->setValue(qRound(layer.position.y()));
  w_->setValue(qRound(layer.size.width()));
  h_->setValue(qRound(layer.size.height()));
  fontSize_->setValue(layer.fontSize);
  enterAnimation_->setCurrentIndex(static_cast<int>(layer.enterAnimation));
  text_->setEnabled(layer.type == LayerType::Text);
}

void DesignPage::applyProperties()
{
  auto &state = AppState::instance();
  auto &project = state.mutableProject();
  if (currentRow_ < 0 || currentRow_ >= project.layers.size()) return;
  auto &layer = project.layers[currentRow_];
  if (layer.type == LayerType::Text) layer.text = text_->text();
  layer.position = QPointF(x_->value(), y_->value());
  layer.size = QSizeF(w_->value(), h_->value());
  layer.fontSize = fontSize_->value();
  layer.enterAnimation = static_cast<AnimationPreset>(enterAnimation_->currentIndex());
  state.notifyModelChanged();
}

void DesignPage::addTextLayer()
{
  auto &state = AppState::instance();
  Layer layer;
  layer.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  layer.name = "Texto nuevo";
  layer.type = LayerType::Text;
  layer.position = {180, 500};
  layer.size = {700, 100};
  layer.text = "NUEVO TEXTO";
  layer.color = Qt::white;
  layer.fontSize = 46;
  state.mutableProject().layers.push_back(layer);
  state.notifyModelChanged();
}

void DesignPage::addShapeLayer()
{
  auto &state = AppState::instance();
  Layer layer;
  layer.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  layer.name = "Forma nueva";
  layer.type = LayerType::Shape;
  layer.position = {150, 650};
  layer.size = {600, 120};
  layer.color = QColor(30, 42, 58, 230);
  state.mutableProject().layers.push_back(layer);
  state.notifyModelChanged();
}

void DesignPage::importPsdPlaceholder()
{
  const QString file = QFileDialog::getOpenFileName(this, "Importar diseño de Photoshop", {}, "Photoshop (*.psd *.psb)");
  if (file.isEmpty()) return;
  QMessageBox::information(this, "Importador PSD", "El archivo fue seleccionado correctamente. En esta base 0.1 el importador está desacoplado del editor; la siguiente implementación conectará el parser PSD para conservar grupos, capas y rasterizar efectos no compatibles.");
}

void DesignPage::refreshCanvas()
{
  const QImage frame = AppState::instance().previewFrame();
  canvas_->setPixmap(QPixmap::fromImage(frame).scaled(canvas_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

} // namespace wg
