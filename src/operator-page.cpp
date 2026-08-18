#include "operator-page.hpp"
#include "app-state.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

namespace wg {

static QLabel *makeScreen(const QString &title, QVBoxLayout *parent)
{
  auto *label = new QLabel();
  label->setObjectName("wgScreen");
  label->setMinimumSize(360, 205);
  label->setAlignment(Qt::AlignCenter);
  label->setScaledContents(false);
  auto *caption = new QLabel(title);
  caption->setObjectName("wgSubtle");
  parent->addWidget(caption);
  parent->addWidget(label, 1);
  return label;
}

OperatorPage::OperatorPage(QWidget *parent) : QWidget(parent)
{
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(18, 18, 18, 18);
  root->setSpacing(14);

  auto *body = new QHBoxLayout();
  body->setSpacing(14);

  auto *leftCard = new QFrame();
  leftCard->setObjectName("wgCard");
  leftCard->setFixedWidth(230);
  auto *left = new QVBoxLayout(leftCard);
  left->addWidget(new QLabel("SERVICIO PREPARADO"));
  serviceList_ = new QListWidget();
  serviceList_->addItems({"Pastor principal", "Juan 3:16", "Tema: El poder de la fe", "Grupo de alabanza", "Romanos 8:28"});
  left->addWidget(serviceList_, 1);
  left->addWidget(new QLabel("BIBLIA"));
  bibleSearch_ = new QLineEdit();
  bibleSearch_->setPlaceholderText("Juan 3:16-18");
  left->addWidget(bibleSearch_);
  auto *bibleHint = new QLabel("Motor bíblico preparado para RVR1960, NVI, NTV y TLA. Los paquetes de texto se conectarán respetando sus licencias.");
  bibleHint->setObjectName("wgSubtle");
  bibleHint->setWordWrap(true);
  left->addWidget(bibleHint);
  body->addWidget(leftCard);

  auto *screensCard = new QFrame();
  screensCard->setObjectName("wgCard");
  auto *screens = new QVBoxLayout(screensCard);
  auto *screenRow = new QHBoxLayout();
  auto *previewCol = new QVBoxLayout();
  auto *programCol = new QVBoxLayout();
  previewScreen_ = makeScreen("PREVIEW", previewCol);
  programScreen_ = makeScreen("PROGRAM", programCol);
  screenRow->addLayout(previewCol, 1);
  screenRow->addLayout(programCol, 1);
  screens->addLayout(screenRow, 1);

  statusLabel_ = new QLabel("● FUERA DEL AIRE");
  statusLabel_->setObjectName("wgSubtle");
  screens->addWidget(statusLabel_);
  body->addWidget(screensCard, 1);
  root->addLayout(body, 1);

  auto *controls = new QHBoxLayout();
  auto *previewButton = new QPushButton("VISTA PREVIA");
  auto *airButton = new QPushButton("ENVIAR AL AIRE");
  airButton->setObjectName("wgPrimary");
  auto *hideButton = new QPushButton("OCULTAR");
  hideButton->setObjectName("wgDanger");
  auto *previousButton = new QPushButton("← ANTERIOR");
  auto *nextButton = new QPushButton("SIGUIENTE →");
  controls->addWidget(previousButton);
  controls->addWidget(previewButton);
  controls->addWidget(airButton);
  controls->addWidget(hideButton);
  controls->addWidget(nextButton);
  root->addLayout(controls);

  auto &state = AppState::instance();
  connect(&state, &AppState::previewChanged, this, &OperatorPage::refreshPreview);
  connect(&state, &AppState::programChanged, this, &OperatorPage::refreshProgram);
  connect(&state, &AppState::onAirChanged, this, [this](bool onAir) {
    statusLabel_->setText(onAir ? "● EN VIVO — WORSHIP GRAPHICS" : "● FUERA DEL AIRE");
  });
  connect(previewButton, &QPushButton::clicked, &state, &AppState::rebuildPreview);
  connect(airButton, &QPushButton::clicked, &state, &AppState::showPreviewOnProgram);
  connect(hideButton, &QPushButton::clicked, &state, &AppState::hideProgram);
  connect(previousButton, &QPushButton::clicked, this, [this] { cycleDemo(-1); });
  connect(nextButton, &QPushButton::clicked, this, [this] { cycleDemo(1); });

  refreshPreview();
  refreshProgram();
}

void OperatorPage::refreshPreview()
{
  const QImage frame = AppState::instance().previewFrame();
  previewScreen_->setPixmap(QPixmap::fromImage(frame).scaled(previewScreen_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void OperatorPage::refreshProgram()
{
  const QImage frame = AppState::instance().programFrame();
  programScreen_->setPixmap(QPixmap::fromImage(frame).scaled(programScreen_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void OperatorPage::cycleDemo(int direction)
{
  static const QList<QPair<QString, QString>> people = {
    {"PASTOR CARLOS LÓPEZ", "PASTOR PRINCIPAL"},
    {"MINISTERIO DE ALABANZA", "WORSHIP TEAM"},
    {"PASTOR INVITADO", "CONFERENCISTA"}
  };
  demoIndex_ = (demoIndex_ + direction + people.size()) % people.size();
  auto &state = AppState::instance();
  for (int i = 0; i < state.mutableProject().layers.size(); ++i) {
    auto &layer = state.mutableProject().layers[i];
    if (layer.name == "{{NOMBRE}}") layer.text = people[demoIndex_].first;
    if (layer.name == "{{CARGO}}") layer.text = people[demoIndex_].second;
  }
  state.notifyModelChanged();
}

} // namespace wg
