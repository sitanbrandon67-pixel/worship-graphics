#include "operator-page.hpp"

#include "app-state.hpp"
#include "output-source.hpp"
#include "template-factory.hpp"
#include "template-library.hpp"
#include "theme.hpp"

#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QShowEvent>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

namespace wg {

namespace {
QString defaultBuiltinId(const QString &kind)
{
  if (kind == "scripture") return "builtin:scripture";
  if (kind == "sermon") return "builtin:sermon";
  if (kind == "worship") return "builtin:worship";
  if (kind == "announcement") return "builtin:announcement";
  return "builtin:pastor";
}

QString builtinName(const QString &id)
{
  if (id == "builtin:pastor") return "Pastor Clean";
  if (id == "builtin:motion") return "Motion Pieces";
  if (id == "builtin:scripture") return "Versículo";
  if (id == "builtin:sermon") return "Tema de prédica";
  if (id == "builtin:worship") return "Alabanza";
  if (id == "builtin:announcement") return "Anuncio";
  return "Plantilla";
}

Project builtinProject(const QString &id)
{
  if (id == "builtin:motion") return TemplateFactory::motionPiecesLowerThird();
  if (id == "builtin:scripture") return TemplateFactory::scriptureLowerThird();
  if (id == "builtin:sermon") return TemplateFactory::sermonTitleLowerThird();
  if (id == "builtin:worship") return TemplateFactory::worshipLowerThird();
  if (id == "builtin:announcement") return TemplateFactory::announcementLowerThird();
  return TemplateFactory::pastorLowerThird();
}

bool fillBibleFields(Project &project, const QString &verse, const QString &reference)
{
  bool foundVerse = false;
  bool foundReference = false;
  for (auto &layer : project.layers) {
    if (layer.name == "{{VERSICULO}}") {
      layer.text = verse;
      foundVerse = true;
    }
    if (layer.name == "{{REFERENCIA}}") {
      layer.text = reference;
      foundReference = true;
    }
  }
  if (foundVerse && foundReference) {
    project.usage = TemplateUsage::BibleText;
    project.name = "Versículo · " + reference;
  }
  return foundVerse && foundReference;
}
} // namespace

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

  auto *miniButton = new QPushButton("MINI");
  miniButton->setObjectName("wgSoftButton");
  miniButton->setCheckable(true);
  miniButton->setToolTip("Oculta el monitor y las herramientas para dejar solo los controles en vivo");

  programHeader->addWidget(programTitle);
  programHeader->addSpacing(8);
  programHeader->addWidget(programNameLabel_);
  programHeader->addStretch();
  programHeader->addWidget(miniButton);
  programHeader->addWidget(statusLabel_);
  programLayout->addLayout(programHeader);

  programScreen_ = new QLabel();
  programScreen_->setObjectName("wgScreen");
  programScreen_->setAlignment(Qt::AlignCenter);
  programScreen_->setMinimumHeight(115);
  programScreen_->setMaximumHeight(180);
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

  auto *serviceTop = new QGridLayout();
  serviceType_ = new QComboBox();
  serviceType_->addItem("Pastor / Persona", "pastor");
  serviceType_->addItem("Versículo", "scripture");
  serviceType_->addItem("Tema de prédica", "sermon");
  serviceType_->addItem("Alabanza", "worship");
  serviceType_->addItem("Anuncio", "announcement");
  serviceType_->addItem("Diseño actual", "current");

  templateChoice_ = new QComboBox();
  templateChoice_->setToolTip("Plantilla que se guardará dentro de este cintillo del Servicio preparado");

  auto *typeLabel = new QLabel("Tipo");
  typeLabel->setObjectName("wgSubtle");
  auto *templateLabel = new QLabel("Plantilla");
  templateLabel->setObjectName("wgSubtle");

  auto *addButton = new QPushButton("+ AGREGAR");
  addButton->setObjectName("wgPrimary");

  serviceTop->addWidget(typeLabel, 0, 0);
  serviceTop->addWidget(serviceType_, 0, 1);
  serviceTop->addWidget(templateLabel, 1, 0);
  serviceTop->addWidget(templateChoice_, 1, 1);
  serviceTop->addWidget(addButton, 0, 2, 2, 1);
  serviceTop->setColumnStretch(1, 1);
  service->addLayout(serviceTop);

  serviceList_ = new QListWidget();
  serviceList_->setObjectName("wgPreparedList");
  serviceList_->setMinimumHeight(95);
  serviceList_->setMaximumHeight(180);
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

  auto *bibleTemplateRow = new QHBoxLayout();
  auto *bibleTemplateLabel = new QLabel("Plantilla");
  bibleTemplateLabel->setObjectName("wgSubtle");
  bibleTemplateChoice_ = new QComboBox();
  bibleTemplateChoice_->setToolTip("Plantilla bíblica que se usará para PREPARAR o + A SERVICIO");
  bibleTemplateRow->addWidget(bibleTemplateLabel);
  bibleTemplateRow->addWidget(bibleTemplateChoice_, 1);
  bibleLayout->addLayout(bibleTemplateRow);

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

  connect(serviceType_, &QComboBox::currentIndexChanged, this, &OperatorPage::refreshTemplateChoices);
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

  connect(tabs_, &QTabWidget::currentChanged, this, [this](int) { refreshTemplateChoices(); });

  connect(miniButton, &QPushButton::toggled, this, [this, miniButton](bool mini) {
    programScreen_->setVisible(!mini);
    tabs_->setVisible(!mini);
    miniButton->setText(mini ? "COMPLETO" : "MINI");
    setMinimumHeight(mini ? 165 : 420);
    setMinimumWidth(mini ? 330 : 410);
  });

  refreshTemplateChoices();
  seedPreparedService();
  refreshProgram();

  QTimer::singleShot(0, this, &OperatorPage::tryLoadInstalledBible);
}


void OperatorPage::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  refreshTemplateChoices();
}

void OperatorPage::seedPreparedService()
{
  prepared_.clear();
  const QStringList kinds = {"pastor", "scripture", "sermon", "worship", "announcement"};
  for (const QString &kind : kinds) {
    QString id = TemplateLibrary::defaultTemplate(kind);
    if (id.isEmpty()) id = defaultBuiltinId(kind);
    Project p = projectForTemplate(kind, id);
    appendPrepared(p.name, kind, p, false);
  }
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

void OperatorPage::populateTemplateCombo(QComboBox *combo, const QString &kind) const
{
  if (!combo) return;
  combo->blockSignals(true);
  combo->clear();

  if (kind == "current") {
    combo->addItem("Diseño actual", "current");
    combo->setEnabled(false);
    combo->blockSignals(false);
    return;
  }
  combo->setEnabled(true);

  if (kind == "pastor") {
    combo->addItem("Integrada · Pastor Clean", "builtin:pastor");
    combo->addItem("Integrada · Motion Pieces", "builtin:motion");
  } else {
    const QString builtin = defaultBuiltinId(kind);
    combo->addItem("Integrada · " + builtinName(builtin), builtin);
  }

  for (const TemplateEntry &entry : TemplateLibrary::entries()) {
    if (kind == "scripture") {
      if (entry.usage != TemplateUsage::BibleText) continue;
    } else {
      if (entry.usage == TemplateUsage::BibleText) continue;
    }
    combo->addItem("Mi plantilla · " + entry.name, entry.filePath);
  }

  QString preferred = TemplateLibrary::defaultTemplate(kind);
  if (preferred.isEmpty()) preferred = defaultBuiltinId(kind);
  const int preferredIndex = combo->findData(preferred);
  combo->setCurrentIndex(preferredIndex >= 0 ? preferredIndex : 0);
  combo->blockSignals(false);
}

void OperatorPage::refreshTemplateChoices()
{
  const QString kind = serviceType_ ? serviceType_->currentData().toString() : QString("pastor");
  populateTemplateCombo(templateChoice_, kind);
  populateTemplateCombo(bibleTemplateChoice_, "scripture");
}

Project OperatorPage::projectForTemplate(const QString &kind, const QString &templateId) const
{
  if (kind == "current") return AppState::instance().project();

  Project project;
  if (templateId.startsWith("builtin:")) {
    project = builtinProject(templateId);
  } else {
    QString error;
    if (!TemplateLibrary::load(templateId, &project, &error)) {
      project = builtinProject(defaultBuiltinId(kind));
    }
  }

  if (kind == "scripture") {
    const QString verseText = currentPassage_.valid
        ? currentPassage_.text
        : QString("Porque de tal manera amó Dios al mundo...");
    const QString referenceText = currentPassage_.valid
        ? currentPassage_.reference
        : QString("Juan 3:16");
    if (!fillBibleFields(project, verseText, referenceText))
      project = TemplateFactory::scriptureLowerThird(verseText, referenceText);
  }
  return project;
}

Project OperatorPage::projectForServiceKind(const QString &kind) const
{
  if (kind == "current") return AppState::instance().project();
  QString id = templateChoice_ ? templateChoice_->currentData().toString() : QString();
  if (id.isEmpty()) {
    id = TemplateLibrary::defaultTemplate(kind);
    if (id.isEmpty()) id = defaultBuiltinId(kind);
  }
  return projectForTemplate(kind, id);
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
  if (kind == "current") {
    label = "Diseño actual · " + project.name;
  } else if (templateChoice_ && templateChoice_->currentIndex() >= 0) {
    label = serviceType_->currentText() + " · " + templateChoice_->currentText().replace("Integrada · ", "").replace("Mi plantilla · ", "");
  }

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

  const QString id = bibleTemplateChoice_ && bibleTemplateChoice_->currentIndex() >= 0
      ? bibleTemplateChoice_->currentData().toString()
      : defaultBuiltinId("scripture");
  AppState::instance().loadProject(projectForTemplate("scripture", id));
  preparedLabel_->setText("PRÓXIMO · " + currentPassage_.reference + " · aún no está al aire");
}

void OperatorPage::addBibleToService()
{
  if (!currentPassage_.valid)
    return;

  const QString id = bibleTemplateChoice_ && bibleTemplateChoice_->currentIndex() >= 0
      ? bibleTemplateChoice_->currentData().toString()
      : defaultBuiltinId("scripture");
  Project project = projectForTemplate("scripture", id);
  QString templateName = bibleTemplateChoice_ ? bibleTemplateChoice_->currentText() : QString("Versículo");
  templateName.replace("Integrada · ", "");
  templateName.replace("Mi plantilla · ", "");

  appendPrepared("Versículo · " + currentPassage_.reference + " · " + templateName,
                 "scripture", project, true);

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
