#include "operator-page.hpp"

#include "app-state.hpp"
#include "output-source.hpp"
#include "template-factory.hpp"
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
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

namespace wg {

OperatorPage::OperatorPage(QWidget *parent) : QWidget(parent)
{
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(12, 10, 12, 10);
  root->setSpacing(9);

  // PROGRAM - compact and always visible.
  auto *programCard = new QFrame();
  programCard->setObjectName("wgCard");
  applySoftShadow(programCard);

  auto *programLayout = new QVBoxLayout(programCard);
  programLayout->setContentsMargins(12, 10, 12, 10);
  programLayout->setSpacing(7);

  auto *programHeader = new QHBoxLayout();
  auto *programTitle = new QLabel("PROGRAM");
  programTitle->setObjectName("wgSectionTitle");

  programNameLabel_ = new QLabel("Sin gráfico al aire");
  programNameLabel_->setObjectName("wgSubtle");

  statusLabel_ = new QLabel("● FUERA DEL AIRE");
  statusLabel_->setObjectName("wgSubtle");

  programHeader->addWidget(programTitle);
  programHeader->addSpacing(8);
  programHeader->addWidget(programNameLabel_);
  programHeader->addStretch();
  programHeader->addWidget(statusLabel_);
  programLayout->addLayout(programHeader);

  programScreen_ = new QLabel();
  programScreen_->setObjectName("wgScreen");
  programScreen_->setAlignment(Qt::AlignCenter);
  programScreen_->setMinimumHeight(155);
  programScreen_->setMaximumHeight(230);
  programLayout->addWidget(programScreen_);

  root->addWidget(programCard);

  // Main live controls.
  auto *controlsCard = new QFrame();
  controlsCard->setObjectName("wgFloatingBar");
  applySoftShadow(controlsCard);

  auto *controls = new QHBoxLayout(controlsCard);
  controls->setContentsMargins(10, 7, 10, 7);
  controls->setSpacing(7);

  auto *previousButton = new QPushButton("←");
  previousButton->setToolTip("Preparar el cintillo anterior");

  auto *airButton = new QPushButton("ENVIAR AL AIRE");
  airButton->setObjectName("wgPrimary");

  auto *hideButton = new QPushButton("OCULTAR");
  hideButton->setObjectName("wgDanger");

  auto *nextButton = new QPushButton("→");
  nextButton->setToolTip("Preparar el siguiente cintillo");

  controls->addWidget(previousButton);
  controls->addWidget(airButton, 1);
  controls->addWidget(hideButton);
  controls->addWidget(nextButton);

  root->addWidget(controlsCard);

  // Tabs keep the operator dock small. Only one tool section is open at a time.
  tabs_ = new QTabWidget();
  tabs_->setObjectName("wgOperatorTabs");

  // -----------------------------------------------------------
  // SERVICE PREPARED TAB
  // -----------------------------------------------------------
  auto *servicePage = new QWidget();
  auto *service = new QVBoxLayout(servicePage);
  service->setContentsMargins(8, 8, 8, 8);
  service->setSpacing(7);

  auto *serviceTop = new QHBoxLayout();
  serviceType_ = new QComboBox();
  serviceType_->addItem("Pastor / Persona", "pastor");
  serviceType_->addItem("Versículo", "scripture");
  serviceType_->addItem("Tema de prédica", "sermon");
  serviceType_->addItem("Alabanza", "worship");
  serviceType_->addItem("Anuncio", "announcement");
  serviceType_->addItem("Diseño actual", "current");

  auto *addButton = new QPushButton("+ AGREGAR");
  addButton->setObjectName("wgPrimary");

  serviceTop->addWidget(serviceType_, 1);
  serviceTop->addWidget(addButton);
  service->addLayout(serviceTop);

  serviceList_ = new QListWidget();
  serviceList_->setObjectName("wgPreparedList");
  serviceList_->setMinimumHeight(125);
  serviceList_->setMaximumHeight(215);
  service->addWidget(serviceList_, 1);

  auto *serviceActions = new QHBoxLayout();
  auto *upButton = new QPushButton("↑");
  auto *downButton = new QPushButton("↓");
  auto *removeButton = new QPushButton("ELIMINAR");
  removeButton->setObjectName("wgDanger");

  serviceActions->addWidget(upButton);
  serviceActions->addWidget(downButton);
  serviceActions->addStretch();
  serviceActions->addWidget(removeButton);
  service->addLayout(serviceActions);

  preparedLabel_ = new QLabel("Selecciona un cintillo para prepararlo");
  preparedLabel_->setObjectName("wgSubtle");
  preparedLabel_->setWordWrap(true);
  service->addWidget(preparedLabel_);

  tabs_->addTab(servicePage, "SERVICIO");

  // -----------------------------------------------------------
  // BIBLE TAB
  // -----------------------------------------------------------
  auto *biblePage = new QWidget();
  auto *bibleLayout = new QVBoxLayout(biblePage);
  bibleLayout->setContentsMargins(8, 8, 8, 8);
  bibleLayout->setSpacing(7);

  auto *bibleHeader = new QHBoxLayout();
  bibleStatus_ = new QLabel("RVR1960 · importa tu XML una vez");
  bibleStatus_->setObjectName("wgSubtle");

  auto *install = new QPushButton("IMPORTAR XML");
  install->setObjectName("wgSoftButton");

  bibleHeader->addWidget(bibleStatus_, 1);
  bibleHeader->addWidget(install);
  bibleLayout->addLayout(bibleHeader);

  auto *searchRow = new QHBoxLayout();
  bibleSearch_ = new QLineEdit();
  bibleSearch_->setPlaceholderText("Juan 3:16 · Salmos 23");

  auto *searchButton = new QPushButton("BUSCAR");
  auto *prevVerse = new QPushButton("←");
  auto *nextVerse = new QPushButton("→");

  searchRow->addWidget(bibleSearch_, 1);
  searchRow->addWidget(searchButton);
  searchRow->addWidget(prevVerse);
  searchRow->addWidget(nextVerse);
  bibleLayout->addLayout(searchRow);

  auto *selectors = new QGridLayout();
  book_ = new QComboBox();
  chapter_ = new QComboBox();
  verse_ = new QComboBox();

  selectors->addWidget(new QLabel("Libro"), 0, 0);
  selectors->addWidget(new QLabel("Cap."), 0, 1);
  selectors->addWidget(new QLabel("Vers."), 0, 2);
  selectors->addWidget(book_, 1, 0);
  selectors->addWidget(chapter_, 1, 1);
  selectors->addWidget(verse_, 1, 2);
  bibleLayout->addLayout(selectors);

  auto *selectButton = new QPushButton("CARGAR");
  bibleLayout->addWidget(selectButton);

  bibleResult_ = new QTextEdit();
  bibleResult_->setReadOnly(true);
  bibleResult_->setPlaceholderText("El texto bíblico aparecerá aquí.");
  bibleResult_->setMinimumHeight(88);
  bibleResult_->setMaximumHeight(130);
  bibleLayout->addWidget(bibleResult_);

  auto *bibleActions = new QHBoxLayout();
  auto *prepare = new QPushButton("PREPARAR");
  prepare->setObjectName("wgPrimary");

  auto *addBible = new QPushButton("+ A SERVICIO");

  bibleActions->addWidget(prepare, 1);
  bibleActions->addWidget(addBible);
  bibleLayout->addLayout(bibleActions);

  tabs_->addTab(biblePage, "BIBLIA");
  root->addWidget(tabs_, 1);

  auto &state = AppState::instance();

  connect(&state, &AppState::programChanged, this, &OperatorPage::refreshProgram);
  connect(&state, &AppState::onAirChanged, this, [this](bool onAir) {
    statusLabel_->setText(onAir ? "● EN VIVO" : "● FUERA DEL AIRE");
    if (!onAir)
      programNameLabel_->setText("Sin gráfico al aire");
  });

  connect(airButton, &QPushButton::clicked, this, [this, &state] {
    ensureOutputInRelevantScenes();

    const int row = serviceList_->currentRow();
    if (row >= 0 && row < prepared_.size())
      programNameLabel_->setText(prepared_[row].label);
    else
      programNameLabel_->setText(state.project().name);

    state.showPreviewOnProgram();
  });

  connect(hideButton, &QPushButton::clicked, &state, &AppState::hideProgram);

  connect(previousButton, &QPushButton::clicked, this, &OperatorPage::previousPrepared);
  connect(nextButton, &QPushButton::clicked, this, &OperatorPage::nextPrepared);

  connect(serviceList_, &QListWidget::currentRowChanged, this, &OperatorPage::loadPreparedSelection);
  connect(addButton, &QPushButton::clicked, this, &OperatorPage::addPreparedGraphic);
  connect(removeButton, &QPushButton::clicked, this, &OperatorPage::removePreparedGraphic);
  connect(upButton, &QPushButton::clicked, this, &OperatorPage::movePreparedUp);
  connect(downButton, &QPushButton::clicked, this, &OperatorPage::movePreparedDown);

  connect(install, &QPushButton::clicked, this, &OperatorPage::installBible);
  connect(searchButton, &QPushButton::clicked, this, &OperatorPage::searchBible);
  connect(bibleSearch_, &QLineEdit::returnPressed, this, &OperatorPage::searchBible);
  connect(prepare, &QPushButton::clicked, this, &OperatorPage::prepareBibleForProgram);
  connect(addBible, &QPushButton::clicked, this, &OperatorPage::addBibleToService);

  connect(prevVerse, &QPushButton::clicked, this, [this] { navigateBible(-1); });
  connect(nextVerse, &QPushButton::clicked, this, [this] { navigateBible(1); });

  connect(book_, &QComboBox::currentIndexChanged, this, &OperatorPage::refreshChapters);
  connect(chapter_, &QComboBox::currentIndexChanged, this, &OperatorPage::refreshVerses);
  connect(selectButton, &QPushButton::clicked, this, &OperatorPage::selectBibleVerse);

  seedPreparedService();
  refreshProgram();

  QTimer::singleShot(0, this, &OperatorPage::tryLoadInstalledBible);
}

void OperatorPage::seedPreparedService()
{
  prepared_.clear();
  prepared_.push_back({"Pastor principal", "pastor", TemplateFactory::pastorLowerThird()});
  prepared_.push_back({"Versículo", "scripture", TemplateFactory::scriptureLowerThird()});
  prepared_.push_back({"Tema de prédica", "sermon", TemplateFactory::sermonTitleLowerThird()});
  prepared_.push_back({"Grupo de alabanza", "worship", TemplateFactory::worshipLowerThird()});
  prepared_.push_back({"Anuncio", "announcement", TemplateFactory::announcementLowerThird()});

  rebuildPreparedList(0);
}

void OperatorPage::appendPrepared(const QString &label, const QString &kind, const Project &project, bool select)
{
  PreparedGraphic item;
  item.label = label;
  item.kind = kind;
  item.project = project;

  prepared_.push_back(item);
  rebuildPreparedList(select ? prepared_.size() - 1 : serviceList_->currentRow());
}

void OperatorPage::rebuildPreparedList(int selectedRow)
{
  serviceList_->blockSignals(true);
  serviceList_->clear();

  for (int i = 0; i < prepared_.size(); ++i) {
    const auto &entry = prepared_[i];
    serviceList_->addItem(QString("%1.  %2").arg(i + 1).arg(entry.label));
  }

  serviceList_->blockSignals(false);

  if (prepared_.isEmpty()) {
    preparedLabel_->setText("Servicio preparado vacío · usa + AGREGAR");
    return;
  }

  selectedRow = qBound(0, selectedRow < 0 ? 0 : selectedRow, prepared_.size() - 1);
  serviceList_->setCurrentRow(selectedRow);
}

Project OperatorPage::projectForServiceKind(const QString &kind) const
{
  if (kind == "pastor")
    return TemplateFactory::pastorLowerThird();

  if (kind == "scripture") {
    if (currentPassage_.valid)
      return TemplateFactory::scriptureLowerThird(currentPassage_.text, currentPassage_.reference);
    return TemplateFactory::scriptureLowerThird();
  }

  if (kind == "sermon")
    return TemplateFactory::sermonTitleLowerThird();

  if (kind == "worship")
    return TemplateFactory::worshipLowerThird();

  if (kind == "announcement")
    return TemplateFactory::announcementLowerThird();

  return AppState::instance().project();
}

void OperatorPage::loadPreparedSelection(int row)
{
  if (row < 0 || row >= prepared_.size())
    return;

  AppState::instance().loadProject(prepared_[row].project);
  preparedLabel_->setText("PRÓXIMO · " + prepared_[row].label + " · listo para ENVIAR AL AIRE");
}

void OperatorPage::addPreparedGraphic()
{
  const QString kind = serviceType_->currentData().toString();
  Project project = projectForServiceKind(kind);

  QString label = project.name;
  if (kind == "current")
    label = "Diseño actual · " + project.name;

  appendPrepared(label, kind, project, true);
}

void OperatorPage::removePreparedGraphic()
{
  const int row = serviceList_->currentRow();
  if (row < 0 || row >= prepared_.size())
    return;

  prepared_.removeAt(row);

  if (prepared_.isEmpty()) {
    rebuildPreparedList(-1);
    return;
  }

  rebuildPreparedList(qMin(row, prepared_.size() - 1));
}

void OperatorPage::movePreparedUp()
{
  const int row = serviceList_->currentRow();
  if (row <= 0 || row >= prepared_.size())
    return;

  prepared_.move(row, row - 1);
  rebuildPreparedList(row - 1);
}

void OperatorPage::movePreparedDown()
{
  const int row = serviceList_->currentRow();
  if (row < 0 || row >= prepared_.size() - 1)
    return;

  prepared_.move(row, row + 1);
  rebuildPreparedList(row + 1);
}

void OperatorPage::previousPrepared()
{
  if (prepared_.isEmpty())
    return;

  int row = serviceList_->currentRow();
  if (row < 0)
    row = 0;
  else
    row = (row - 1 + prepared_.size()) % prepared_.size();

  serviceList_->setCurrentRow(row);
}

void OperatorPage::nextPrepared()
{
  if (prepared_.isEmpty())
    return;

  int row = serviceList_->currentRow();
  if (row < 0)
    row = 0;
  else
    row = (row + 1) % prepared_.size();

  serviceList_->setCurrentRow(row);
}

void OperatorPage::tryLoadInstalledBible()
{
  QString error;
  if (bible_.loadInstalled(&error)) {
    bibleStatus_->setText("● " + bible_.translationName() + " · OFFLINE");
    refreshBibleSelectors();
  } else {
    bibleStatus_->setText("RVR1960 · importa tu XML una vez");
  }
}

void OperatorPage::installBible()
{
  const QString file = QFileDialog::getOpenFileName(this, "Importar Biblia RVR1960", {}, "Biblia XML (*.xml)");
  if (file.isEmpty())
    return;

  QString error;
  if (!bible_.installFromXml(file, &error)) {
    bibleStatus_->setText("Error: " + error);
    return;
  }

  bibleStatus_->setText("● " + bible_.translationName() + " · OFFLINE");
  refreshBibleSelectors();
  bibleSearch_->setFocus();
}

void OperatorPage::refreshBibleSelectors()
{
  book_->blockSignals(true);
  book_->clear();
  book_->addItems(bible_.bookNames());
  book_->blockSignals(false);
  refreshChapters();
}

void OperatorPage::refreshChapters()
{
  chapter_->blockSignals(true);
  chapter_->clear();

  const int count = bible_.chapterCount(book_->currentIndex());
  for (int i = 1; i <= count; ++i)
    chapter_->addItem(QString::number(i));

  chapter_->blockSignals(false);
  refreshVerses();
}

void OperatorPage::refreshVerses()
{
  verse_->clear();
  if (chapter_->currentIndex() < 0)
    return;

  const int count = bible_.verseCount(book_->currentIndex(), chapter_->currentIndex() + 1);
  for (int i = 1; i <= count; ++i)
    verse_->addItem(QString::number(i));
}

void OperatorPage::searchBible()
{
  if (!bible_.isLoaded()) {
    bibleStatus_->setText("Importa primero el XML RVR1960.");
    return;
  }

  showPassage(bible_.search(bibleSearch_->text()));
}

void OperatorPage::selectBibleVerse()
{
  if (!bible_.isLoaded() || book_->currentIndex() < 0 ||
      chapter_->currentIndex() < 0 || verse_->currentIndex() < 0)
    return;

  showPassage(bible_.passage(book_->currentIndex(),
                             chapter_->currentIndex() + 1,
                             verse_->currentIndex() + 1,
                             verse_->currentIndex() + 1));
}

void OperatorPage::showPassage(const BiblePassage &passage)
{
  if (!passage.valid) {
    bibleResult_->setPlainText("No se encontró la referencia. Ejemplo: Juan 3:16 o Salmos 23.");
    return;
  }

  currentPassage_ = passage;
  bibleResult_->setPlainText(passage.reference + "\n\n" + passage.text);

  book_->setCurrentIndex(passage.bookIndex);
  chapter_->setCurrentIndex(passage.chapter - 1);

  if (passage.verseStart > 0)
    verse_->setCurrentIndex(passage.verseStart - 1);
}

void OperatorPage::navigateBible(int delta)
{
  if (!currentPassage_.valid) {
    searchBible();
    return;
  }

  showPassage(bible_.adjacent(currentPassage_, delta));
}

void OperatorPage::prepareBibleForProgram()
{
  if (!currentPassage_.valid)
    return;

  AppState::instance().loadProject(
      TemplateFactory::scriptureLowerThird(currentPassage_.text, currentPassage_.reference));

  preparedLabel_->setText("PRÓXIMO · " + currentPassage_.reference + " · aún no está al aire");
}

void OperatorPage::addBibleToService()
{
  if (!currentPassage_.valid)
    return;

  appendPrepared("Versículo · " + currentPassage_.reference,
                 "scripture",
                 TemplateFactory::scriptureLowerThird(currentPassage_.text, currentPassage_.reference),
                 true);

  tabs_->setCurrentIndex(0);
}

void OperatorPage::refreshProgram()
{
  const QImage frame = AppState::instance().programFrame();

  if (frame.isNull()) {
    programScreen_->clear();
    return;
  }

  programScreen_->setPixmap(
      QPixmap::fromImage(frame).scaled(programScreen_->size(),
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
}

} // namespace wg
