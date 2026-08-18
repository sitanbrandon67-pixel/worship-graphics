#include "operator-page.hpp"
#include "app-state.hpp"
#include "theme.hpp"

#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

namespace wg {

static QLabel *makeScreen(const QString &title, QVBoxLayout *parent)
{
  auto *caption = new QLabel(title); caption->setObjectName("wgSectionTitle"); parent->addWidget(caption);
  auto *label = new QLabel(); label->setObjectName("wgScreen"); label->setMinimumSize(360, 205); label->setAlignment(Qt::AlignCenter); parent->addWidget(label, 1);
  return label;
}

OperatorPage::OperatorPage(QWidget *parent) : QWidget(parent)
{
  auto *root = new QVBoxLayout(this); root->setContentsMargins(20, 18, 20, 18); root->setSpacing(14);
  auto *body = new QHBoxLayout(); body->setSpacing(14);

  auto *leftColumn = new QVBoxLayout(); leftColumn->setSpacing(12);
  auto *serviceCard = new QFrame(); serviceCard->setObjectName("wgCard"); applySoftShadow(serviceCard);
  auto *service = new QVBoxLayout(serviceCard);
  auto *serviceTitle = new QLabel("SERVICIO PREPARADO"); serviceTitle->setObjectName("wgSectionTitle"); service->addWidget(serviceTitle);
  serviceList_ = new QListWidget(); serviceList_->addItems({"Pastor principal", "Versículo", "Tema de prédica", "Grupo de alabanza", "Anuncio"}); serviceList_->setMaximumHeight(150); service->addWidget(serviceList_);
  leftColumn->addWidget(serviceCard);

  auto *bibleCard = new QFrame(); bibleCard->setObjectName("wgCard"); applySoftShadow(bibleCard);
  auto *bibleLayout = new QVBoxLayout(bibleCard);
  auto *bibleHeader = new QHBoxLayout(); auto *bibleTitle = new QLabel("BIBLIA OFFLINE"); bibleTitle->setObjectName("wgSectionTitle");
  auto *install = new QPushButton("IMPORTAR XML"); install->setObjectName("wgSoftButton"); bibleHeader->addWidget(bibleTitle); bibleHeader->addStretch(); bibleHeader->addWidget(install); bibleLayout->addLayout(bibleHeader);
  bibleStatus_ = new QLabel("RVR1960 no instalada"); bibleStatus_->setObjectName("wgSubtle"); bibleLayout->addWidget(bibleStatus_);

  bibleSearch_ = new QLineEdit(); bibleSearch_->setPlaceholderText("Juan 3:16 · Jn 3 16 · Salmos 23"); bibleLayout->addWidget(bibleSearch_);
  auto *searchRow = new QHBoxLayout(); auto *searchButton = new QPushButton("BUSCAR"); auto *prevVerse = new QPushButton("←"); auto *nextVerse = new QPushButton("→");
  searchRow->addWidget(searchButton, 1); searchRow->addWidget(prevVerse); searchRow->addWidget(nextVerse); bibleLayout->addLayout(searchRow);

  auto *selectors = new QGridLayout();
  book_ = new QComboBox(); chapter_ = new QComboBox(); verse_ = new QComboBox();
  selectors->addWidget(new QLabel("Libro"), 0, 0); selectors->addWidget(new QLabel("Cap."), 0, 1); selectors->addWidget(new QLabel("Vers."), 0, 2);
  selectors->addWidget(book_, 1, 0); selectors->addWidget(chapter_, 1, 1); selectors->addWidget(verse_, 1, 2); bibleLayout->addLayout(selectors);
  auto *selectButton = new QPushButton("CARGAR SELECCIÓN"); bibleLayout->addWidget(selectButton);

  bibleResult_ = new QTextEdit(); bibleResult_->setReadOnly(true); bibleResult_->setPlaceholderText("El texto bíblico aparecerá aquí."); bibleResult_->setMinimumHeight(125); bibleLayout->addWidget(bibleResult_);
  auto *use = new QPushButton("USAR EN PREVIEW"); use->setObjectName("wgPrimary"); bibleLayout->addWidget(use);
  leftColumn->addWidget(bibleCard, 1);
  body->addLayout(leftColumn, 0);

  auto *screensCard = new QFrame(); screensCard->setObjectName("wgCard"); applySoftShadow(screensCard);
  auto *screens = new QVBoxLayout(screensCard);
  auto *screenRow = new QHBoxLayout(); screenRow->setSpacing(12);
  auto *previewCol = new QVBoxLayout(); auto *programCol = new QVBoxLayout();
  previewScreen_ = makeScreen("PREVIEW", previewCol); programScreen_ = makeScreen("PROGRAM", programCol);
  screenRow->addLayout(previewCol, 1); screenRow->addLayout(programCol, 1); screens->addLayout(screenRow, 1);
  statusLabel_ = new QLabel("● FUERA DEL AIRE"); statusLabel_->setObjectName("wgSubtle"); screens->addWidget(statusLabel_);
  body->addWidget(screensCard, 1); root->addLayout(body, 1);

  auto *controlsCard = new QFrame(); controlsCard->setObjectName("wgFloatingBar"); applySoftShadow(controlsCard);
  auto *controls = new QHBoxLayout(controlsCard); controls->setContentsMargins(14, 10, 14, 10);
  auto *previousButton = new QPushButton("← ANTERIOR"); auto *previewButton = new QPushButton("VISTA PREVIA");
  auto *airButton = new QPushButton("ENVIAR AL AIRE"); airButton->setObjectName("wgPrimary");
  auto *hideButton = new QPushButton("OCULTAR"); hideButton->setObjectName("wgDanger"); auto *nextButton = new QPushButton("SIGUIENTE →");
  controls->addWidget(previousButton); controls->addWidget(previewButton); controls->addWidget(airButton); controls->addWidget(hideButton); controls->addWidget(nextButton); root->addWidget(controlsCard);

  auto &state = AppState::instance();
  connect(&state, &AppState::previewChanged, this, &OperatorPage::refreshPreview);
  connect(&state, &AppState::programChanged, this, &OperatorPage::refreshProgram);
  connect(&state, &AppState::onAirChanged, this, [this](bool onAir) { statusLabel_->setText(onAir ? "● EN VIVO — WORSHIP GRAPHICS" : "● FUERA DEL AIRE"); });
  connect(previewButton, &QPushButton::clicked, &state, &AppState::rebuildPreview);
  connect(airButton, &QPushButton::clicked, &state, &AppState::showPreviewOnProgram);
  connect(hideButton, &QPushButton::clicked, &state, &AppState::hideProgram);
  connect(previousButton, &QPushButton::clicked, this, [this] { cycleDemo(-1); }); connect(nextButton, &QPushButton::clicked, this, [this] { cycleDemo(1); });

  connect(install, &QPushButton::clicked, this, &OperatorPage::installBible);
  connect(searchButton, &QPushButton::clicked, this, &OperatorPage::searchBible); connect(bibleSearch_, &QLineEdit::returnPressed, this, &OperatorPage::searchBible);
  connect(use, &QPushButton::clicked, this, &OperatorPage::useBibleOnPreview);
  connect(prevVerse, &QPushButton::clicked, this, [this] { navigateBible(-1); }); connect(nextVerse, &QPushButton::clicked, this, [this] { navigateBible(1); });
  connect(book_, &QComboBox::currentIndexChanged, this, &OperatorPage::refreshChapters); connect(chapter_, &QComboBox::currentIndexChanged, this, &OperatorPage::refreshVerses);
  connect(selectButton, &QPushButton::clicked, this, &OperatorPage::selectBibleVerse);

  refreshPreview(); refreshProgram();
  QTimer::singleShot(0, this, &OperatorPage::tryLoadInstalledBible);
}

void OperatorPage::tryLoadInstalledBible()
{
  QString error;
  if (bible_.loadInstalled(&error)) {
    bibleStatus_->setText("● " + bible_.translationName()); refreshBibleSelectors();
  } else {
    bibleStatus_->setText("RVR1960 · importa tu XML una vez");
  }
}

void OperatorPage::installBible()
{
  const QString file = QFileDialog::getOpenFileName(this, "Importar Biblia RVR1960", {}, "Biblia XML (*.xml)"); if (file.isEmpty()) return;
  QString error;
  if (!bible_.installFromXml(file, &error)) { bibleStatus_->setText("Error: " + error); return; }
  bibleStatus_->setText("● " + bible_.translationName() + " · OFFLINE"); refreshBibleSelectors(); bibleSearch_->setFocus();
}

void OperatorPage::refreshBibleSelectors()
{
  book_->blockSignals(true); book_->clear(); book_->addItems(bible_.bookNames()); book_->blockSignals(false); refreshChapters();
}

void OperatorPage::refreshChapters()
{
  chapter_->blockSignals(true); chapter_->clear(); const int count = bible_.chapterCount(book_->currentIndex()); for (int i = 1; i <= count; ++i) chapter_->addItem(QString::number(i)); chapter_->blockSignals(false); refreshVerses();
}

void OperatorPage::refreshVerses()
{
  verse_->clear(); if (chapter_->currentIndex() < 0) return; const int count = bible_.verseCount(book_->currentIndex(), chapter_->currentIndex() + 1); for (int i = 1; i <= count; ++i) verse_->addItem(QString::number(i));
}

void OperatorPage::searchBible()
{
  if (!bible_.isLoaded()) { bibleStatus_->setText("Importa primero el XML RVR1960."); return; }
  showPassage(bible_.search(bibleSearch_->text()));
}

void OperatorPage::selectBibleVerse()
{
  if (!bible_.isLoaded() || book_->currentIndex() < 0 || chapter_->currentIndex() < 0 || verse_->currentIndex() < 0) return;
  showPassage(bible_.passage(book_->currentIndex(), chapter_->currentIndex() + 1, verse_->currentIndex() + 1, verse_->currentIndex() + 1));
}

void OperatorPage::showPassage(const BiblePassage &passage)
{
  if (!passage.valid) { bibleResult_->setPlainText("No se encontró la referencia. Ejemplo: Juan 3:16 o Salmos 23."); return; }
  currentPassage_ = passage; bibleResult_->setPlainText(passage.reference + "\n\n" + passage.text);
  book_->setCurrentIndex(passage.bookIndex); chapter_->setCurrentIndex(passage.chapter - 1); if (passage.verseStart > 0) verse_->setCurrentIndex(passage.verseStart - 1);
}

void OperatorPage::navigateBible(int delta)
{
  if (!currentPassage_.valid) { searchBible(); return; }
  showPassage(bible_.adjacent(currentPassage_, delta));
}

void OperatorPage::useBibleOnPreview()
{
  if (!currentPassage_.valid) return;
  AppState::instance().applyBiblePassage(currentPassage_.text, currentPassage_.reference);
}

void OperatorPage::refreshPreview()
{
  const QImage frame = AppState::instance().previewFrame(); previewScreen_->setPixmap(QPixmap::fromImage(frame).scaled(previewScreen_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void OperatorPage::refreshProgram()
{
  const QImage frame = AppState::instance().programFrame(); programScreen_->setPixmap(QPixmap::fromImage(frame).scaled(programScreen_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void OperatorPage::cycleDemo(int direction)
{
  static const QList<QPair<QString, QString>> people = {{"PASTOR MOISÉS", "PASTOR PRINCIPAL"}, {"MINISTERIO DE ALABANZA", "WORSHIP TEAM"}, {"PASTOR INVITADO", "CONFERENCISTA"}};
  demoIndex_ = (demoIndex_ + direction + people.size()) % people.size(); auto &state = AppState::instance();
  for (auto &layer : state.mutableProject().layers) { if (layer.name == "{{NOMBRE}}") layer.text = people[demoIndex_].first; if (layer.name == "{{CARGO}}") layer.text = people[demoIndex_].second; }
  state.notifyModelChanged();
}

} // namespace wg
