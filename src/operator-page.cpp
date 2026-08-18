#include "operator-page.hpp"

#include "app-state.hpp"
#include "output-source.hpp"
#include "template-factory.hpp"
#include "template-library.hpp"
#include "theme.hpp"

#include <QAbstractItemView>
#include <QCompleter>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QShowEvent>
#include <QStringListModel>
#include <QStyle>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

namespace wg {

namespace {
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

QPushButton *iconButton(QWidget *owner, QStyle::StandardPixmap icon, const QString &tip)
{
  auto *button = new QPushButton();
  button->setObjectName("wgIconButton");
  button->setIcon(owner->style()->standardIcon(icon));
  button->setIconSize({15, 15});
  button->setToolTip(tip);
  return button;
}

QPushButton *glyphButton(const QString &glyph, const QString &tip)
{
  auto *button = new QPushButton(glyph);
  button->setObjectName("wgIconButton");
  button->setToolTip(tip);
  return button;
}
} // namespace

OperatorPage::OperatorPage(QWidget *parent) : QWidget(parent)
{
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(8, 7, 8, 7);
  root->setSpacing(6);

  auto *programCard = new QFrame();
  programCard->setObjectName("wgCard");
  applySoftShadow(programCard);
  auto *programLayout = new QVBoxLayout(programCard);
  programLayout->setContentsMargins(8, 7, 8, 7);
  programLayout->setSpacing(5);

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

  programHeader->addWidget(programTitle);
  programHeader->addSpacing(6);
  programHeader->addWidget(programNameLabel_, 1);
  programHeader->addWidget(miniButton);
  programHeader->addWidget(statusLabel_);
  programLayout->addLayout(programHeader);

  programScreen_ = new QLabel();
  programScreen_->setObjectName("wgScreen");
  programScreen_->setAlignment(Qt::AlignCenter);
  programScreen_->setMinimumHeight(100);
  programScreen_->setMaximumHeight(165);
  programLayout->addWidget(programScreen_);
  root->addWidget(programCard);

  auto *controlsCard = new QFrame();
  controlsCard->setObjectName("wgFloatingBar");
  auto *controls = new QHBoxLayout(controlsCard);
  controls->setContentsMargins(7, 5, 7, 5);
  controls->setSpacing(5);
  auto *previousButton = glyphButton("←", "Cintillo anterior del servicio");
  auto *airButton = new QPushButton("ENVIAR AL AIRE");
  airButton->setObjectName("wgPrimary");
  auto *hideButton = new QPushButton("OCULTAR");
  hideButton->setObjectName("wgDanger");
  auto *nextButton = glyphButton("→", "Siguiente cintillo del servicio");
  controls->addWidget(previousButton);
  controls->addWidget(airButton, 1);
  controls->addWidget(hideButton);
  controls->addWidget(nextButton);
  root->addWidget(controlsCard);

  tabs_ = new QTabWidget();
  tabs_->setObjectName("wgOperatorTabs");

  // ------------------------- SERVICIO -------------------------
  auto *servicePage = new QWidget();
  auto *service = new QVBoxLayout(servicePage);
  service->setContentsMargins(7, 7, 7, 7);
  service->setSpacing(5);

  auto *templatesHeader = new QHBoxLayout();
  auto *templatesTitle = new QLabel("PLANTILLAS");
  templatesTitle->setObjectName("wgSectionTitle");
  auto *reloadTemplates = iconButton(this, QStyle::SP_BrowserReload, "Actualizar biblioteca");
  auto *addSelected = new QPushButton("+ A SERVICIO");
  addSelected->setObjectName("wgPrimary");
  templatesHeader->addWidget(templatesTitle);
  templatesHeader->addStretch();
  templatesHeader->addWidget(reloadTemplates);
  templatesHeader->addWidget(addSelected);
  service->addLayout(templatesHeader);

  serviceTemplates_ = new QListWidget();
  serviceTemplates_->setSelectionMode(QAbstractItemView::SingleSelection);
  serviceTemplates_->setMinimumHeight(72);
  serviceTemplates_->setMaximumHeight(118);
  serviceTemplates_->setToolTip("Un clic prepara la plantilla. Doble clic la agrega al servicio.");
  service->addWidget(serviceTemplates_);

  auto *queueHeader = new QHBoxLayout();
  auto *queueTitle = new QLabel("SERVICIO");
  queueTitle->setObjectName("wgSectionTitle");
  auto *upButton = iconButton(this, QStyle::SP_ArrowUp, "Subir en el servicio");
  auto *downButton = iconButton(this, QStyle::SP_ArrowDown, "Bajar en el servicio");
  auto *removeButton = iconButton(this, QStyle::SP_TrashIcon, "Eliminar del servicio");
  queueHeader->addWidget(queueTitle);
  queueHeader->addStretch();
  queueHeader->addWidget(upButton);
  queueHeader->addWidget(downButton);
  queueHeader->addWidget(removeButton);
  service->addLayout(queueHeader);

  serviceList_ = new QListWidget();
  serviceList_->setObjectName("wgPreparedList");
  serviceList_->setMinimumHeight(105);
  service->addWidget(serviceList_, 1);

  preparedLabel_ = new QLabel("Servicio vacío · selecciona una plantilla arriba o agrega un versículo desde Biblia");
  preparedLabel_->setObjectName("wgSubtle");
  preparedLabel_->setWordWrap(true);
  service->addWidget(preparedLabel_);

  tabs_->addTab(servicePage, "SERVICIO");

  // -------------------------- BIBLIA --------------------------
  auto *biblePage = new QWidget();
  auto *bibleLayout = new QVBoxLayout(biblePage);
  bibleLayout->setContentsMargins(7, 7, 7, 7);
  bibleLayout->setSpacing(5);

  auto *bibleHeader = new QHBoxLayout();
  bibleStatus_ = new QLabel("RVR1960 · importa tu XML una vez");
  bibleStatus_->setObjectName("wgSubtle");
  bibleTemplateLabel_ = new QLabel();
  bibleTemplateLabel_->setObjectName("wgSubtle");
  auto *install = iconButton(this, QStyle::SP_DialogOpenButton, "Importar Biblia XML");
  bibleHeader->addWidget(bibleStatus_, 1);
  bibleHeader->addWidget(bibleTemplateLabel_);
  bibleHeader->addWidget(install);
  bibleLayout->addLayout(bibleHeader);

  auto *searchRow = new QHBoxLayout();
  bibleSearch_ = new QLineEdit();
  bibleSearch_->setPlaceholderText("deu 6:4 · sal 23 · juan 3:16");
  auto *prevVerse = glyphButton("←", "Versículo anterior");
  auto *nextVerse = glyphButton("→", "Versículo siguiente");
  searchRow->addWidget(bibleSearch_, 1);
  searchRow->addWidget(prevVerse);
  searchRow->addWidget(nextVerse);
  bibleLayout->addLayout(searchRow);

  auto *hint = new QLabel("Escribe parte del libro, Enter completa el nombre; escribe capítulo/versículo y Enter carga.");
  hint->setObjectName("wgSubtle");
  hint->setWordWrap(true);
  bibleLayout->addWidget(hint);

  bibleSuggestionModel_ = new QStringListModel(this);
  bibleCompleter_ = new QCompleter(bibleSuggestionModel_, this);
  bibleCompleter_->setCaseSensitivity(Qt::CaseInsensitive);
  bibleCompleter_->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  bibleCompleter_->setMaxVisibleItems(8);
  bibleSearch_->setCompleter(bibleCompleter_);

  bibleResult_ = new QTextEdit();
  bibleResult_->setReadOnly(true);
  bibleResult_->setPlaceholderText("El versículo aparecerá aquí.");
  bibleResult_->setMinimumHeight(110);
  bibleLayout->addWidget(bibleResult_, 1);

  auto *addBible = new QPushButton("+ A SERVICIO");
  addBible->setObjectName("wgPrimary");
  bibleLayout->addWidget(addBible);

  tabs_->addTab(biblePage, "BIBLIA");
  root->addWidget(tabs_, 1);

  auto &state = AppState::instance();
  connect(&state, &AppState::programChanged, this, &OperatorPage::refreshProgram);
  connect(&state, &AppState::onAirChanged, this, [this](bool onAir) {
    statusLabel_->setText(onAir ? "● EN VIVO" : "● FUERA DEL AIRE");
    if (!onAir) programNameLabel_->setText("Sin gráfico al aire");
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

  connect(reloadTemplates, &QPushButton::clicked, this, &OperatorPage::refreshServiceTemplates);
  connect(serviceTemplates_, &QListWidget::currentRowChanged, this, &OperatorPage::previewServiceTemplate);
  connect(serviceTemplates_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) { addSelectedTemplateToService(); });
  connect(addSelected, &QPushButton::clicked, this, &OperatorPage::addSelectedTemplateToService);
  connect(serviceList_, &QListWidget::currentRowChanged, this, &OperatorPage::loadPreparedSelection);
  connect(removeButton, &QPushButton::clicked, this, &OperatorPage::removePreparedGraphic);
  connect(upButton, &QPushButton::clicked, this, &OperatorPage::movePreparedUp);
  connect(downButton, &QPushButton::clicked, this, &OperatorPage::movePreparedDown);

  connect(install, &QPushButton::clicked, this, &OperatorPage::installBible);
  connect(bibleSearch_, &QLineEdit::textChanged, this, &OperatorPage::updateBibleSuggestions);
  connect(bibleSearch_, &QLineEdit::returnPressed, this, &OperatorPage::handleBibleEnter);
  connect(bibleCompleter_, qOverload<const QString &>(&QCompleter::activated), this, &OperatorPage::acceptBibleSuggestion);
  connect(prevVerse, &QPushButton::clicked, this, [this] { navigateBible(-1); });
  connect(nextVerse, &QPushButton::clicked, this, [this] { navigateBible(1); });
  connect(addBible, &QPushButton::clicked, this, &OperatorPage::addBibleToService);

  connect(tabs_, &QTabWidget::currentChanged, this, [this](int) {
    refreshServiceTemplates();
    refreshBibleTemplateLabel();
  });

  connect(miniButton, &QPushButton::toggled, this, [this, miniButton](bool mini) {
    programScreen_->setVisible(!mini);
    tabs_->setVisible(!mini);
    miniButton->setText(mini ? "COMPLETO" : "MINI");
    setMinimumHeight(mini ? 150 : 390);
    setMinimumWidth(mini ? 320 : 390);
  });

  seedPreparedService();
  refreshServiceTemplates();
  refreshBibleTemplateLabel();
  refreshProgram();
  QTimer::singleShot(0, this, &OperatorPage::tryLoadInstalledBible);
}

void OperatorPage::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  refreshServiceTemplates();
  refreshBibleTemplateLabel();
}

Project OperatorPage::projectForTemplateId(const QString &templateId) const
{
  if (templateId.startsWith("builtin:"))
    return builtinProject(templateId);

  Project project;
  QString error;
  if (TemplateLibrary::load(templateId, &project, &error))
    return project;
  return {};
}

void OperatorPage::refreshServiceTemplates()
{
  if (!serviceTemplates_) return;
  const QString keep = serviceTemplates_->currentItem()
      ? serviceTemplates_->currentItem()->data(Qt::UserRole).toString()
      : QString();

  serviceTemplates_->blockSignals(true);
  serviceTemplates_->clear();

  struct Builtin { const char *id; const char *name; };
  const Builtin builtins[] = {
      {"builtin:pastor", "Pastor Clean"},
      {"builtin:motion", "Motion Pieces"},
      {"builtin:sermon", "Tema"},
      {"builtin:worship", "Alabanza"},
      {"builtin:announcement", "Anuncio"}};

  int restore = -1;
  for (const Builtin &b : builtins) {
    const QString id = QString::fromUtf8(b.id);
    if (TemplateLibrary::isBuiltinHidden(id)) continue;
    auto *item = new QListWidgetItem(QString::fromUtf8(b.name));
    item->setData(Qt::UserRole, id);
    item->setToolTip("Integrada");
    serviceTemplates_->addItem(item);
    if (id == keep) restore = serviceTemplates_->count() - 1;
  }

  for (const TemplateEntry &entry : TemplateLibrary::entries()) {
    if (entry.usage == TemplateUsage::BibleText) continue;
    auto *item = new QListWidgetItem(entry.name);
    item->setData(Qt::UserRole, entry.filePath);
    item->setToolTip("Mi plantilla");
    serviceTemplates_->addItem(item);
    if (entry.filePath == keep) restore = serviceTemplates_->count() - 1;
  }

  serviceTemplates_->blockSignals(false);
  if (restore >= 0) serviceTemplates_->setCurrentRow(restore);
}

void OperatorPage::previewServiceTemplate(int row)
{
  if (row < 0 || row >= serviceTemplates_->count()) return;
  auto *item = serviceTemplates_->item(row);
  const QString id = item->data(Qt::UserRole).toString();
  Project project = projectForTemplateId(id);
  if (project.layers.isEmpty()) return;

  serviceList_->clearSelection();
  AppState::instance().loadProject(project);
  preparedLabel_->setText("Seleccionada · " + project.name + " · pulsa + A SERVICIO o doble clic");
}

void OperatorPage::addSelectedTemplateToService()
{
  auto *item = serviceTemplates_->currentItem();
  if (!item) {
    QMessageBox::information(this, "Servicio", "Selecciona una plantilla de la lista superior.");
    return;
  }

  Project project = projectForTemplateId(item->data(Qt::UserRole).toString());
  if (project.layers.isEmpty()) return;
  appendPrepared(project.name, "template", project, true);
}

void OperatorPage::seedPreparedService()
{
  prepared_.clear();
  rebuildPreparedList(-1);
}

void OperatorPage::appendPrepared(const QString &label, const QString &kind, const Project &project, bool select)
{
  prepared_.push_back({label, kind, project});
  rebuildPreparedList(select ? prepared_.size() - 1 : serviceList_->currentRow());
}

void OperatorPage::rebuildPreparedList(int selectedRow)
{
  serviceList_->blockSignals(true);
  serviceList_->clear();
  for (int i = 0; i < prepared_.size(); ++i)
    serviceList_->addItem(QString("%1.  %2").arg(i + 1).arg(prepared_[i].label));
  serviceList_->blockSignals(false);

  if (prepared_.isEmpty()) {
    preparedLabel_->setText("Servicio vacío · selecciona una plantilla o agrega un versículo desde Biblia");
    return;
  }

  selectedRow = qBound(0, selectedRow < 0 ? 0 : selectedRow, prepared_.size() - 1);
  serviceList_->setCurrentRow(selectedRow);
}

void OperatorPage::loadPreparedSelection(int row)
{
  if (row < 0 || row >= prepared_.size()) return;
  AppState::instance().loadProject(prepared_[row].project);
  preparedLabel_->setText("PRÓXIMO · " + prepared_[row].label);
}

void OperatorPage::removePreparedGraphic()
{
  const int row = serviceList_->currentRow();
  if (row < 0 || row >= prepared_.size()) return;
  prepared_.removeAt(row);
  rebuildPreparedList(prepared_.isEmpty() ? -1 : qMin(row, prepared_.size() - 1));
}

void OperatorPage::movePreparedUp()
{
  const int row = serviceList_->currentRow();
  if (row <= 0 || row >= prepared_.size()) return;
  prepared_.move(row, row - 1);
  rebuildPreparedList(row - 1);
}

void OperatorPage::movePreparedDown()
{
  const int row = serviceList_->currentRow();
  if (row < 0 || row >= prepared_.size() - 1) return;
  prepared_.move(row, row + 1);
  rebuildPreparedList(row + 1);
}

void OperatorPage::previousPrepared()
{
  if (prepared_.isEmpty()) return;
  int row = serviceList_->currentRow();
  row = row < 0 ? 0 : (row - 1 + prepared_.size()) % prepared_.size();
  serviceList_->setCurrentRow(row);
}

void OperatorPage::nextPrepared()
{
  if (prepared_.isEmpty()) return;
  int row = serviceList_->currentRow();
  row = row < 0 ? 0 : (row + 1) % prepared_.size();
  serviceList_->setCurrentRow(row);
}

void OperatorPage::tryLoadInstalledBible()
{
  QString error;
  if (bible_.loadInstalled(&error)) {
    bibleStatus_->setText("● " + bible_.translationName() + " · OFFLINE");
    updateBibleSuggestions(bibleSearch_->text());
  } else {
    bibleStatus_->setText("RVR1960 · importa tu XML una vez");
  }
}

void OperatorPage::installBible()
{
  const QString file = QFileDialog::getOpenFileName(this, "Importar Biblia", {}, "Biblia XML (*.xml)");
  if (file.isEmpty()) return;

  QString error;
  if (!bible_.installFromXml(file, &error)) {
    bibleStatus_->setText("Error: " + error);
    return;
  }

  bibleStatus_->setText("● " + bible_.translationName() + " · OFFLINE");
  bibleSearch_->setFocus();
  updateBibleSuggestions(bibleSearch_->text());
}

void OperatorPage::updateBibleSuggestions(const QString &text)
{
  if (!bibleSuggestionModel_) return;
  const QStringList suggestions = bible_.bookSuggestions(text, 8);
  bibleSuggestionModel_->setStringList(suggestions);
  if (bibleCompleter_ && !text.trimmed().isEmpty() && !suggestions.isEmpty())
    bibleCompleter_->complete();
}

void OperatorPage::acceptBibleSuggestion(const QString &bookName)
{
  if (bookName.isEmpty()) return;
  const QString current = bibleSearch_->text().trimmed();

  int digitIndex = -1;
  for (int i = 0; i < current.size(); ++i) {
    if (current.at(i).isDigit()) {
      digitIndex = i;
      break;
    }
  }

  const QString tail = digitIndex >= 0 ? current.mid(digitIndex).trimmed() : QString();
  const QString next = bookName + (tail.isEmpty() ? QString(" ") : QString(" ") + tail);
  bibleSearch_->setText(next);
  bibleSearch_->setCursorPosition(next.size());
}

void OperatorPage::handleBibleEnter()
{
  if (!bible_.isLoaded()) {
    bibleStatus_->setText("Importa primero la Biblia XML.");
    return;
  }

  if (bibleCompleter_ && bibleCompleter_->popup()->isVisible() && bibleCompleter_->popup()->currentIndex().isValid()) {
    const QString suggestion = bibleCompleter_->popup()->currentIndex().data().toString();
    if (!suggestion.isEmpty()) {
      acceptBibleSuggestion(suggestion);
      bibleCompleter_->popup()->hide();
      return;
    }
  }

  const QString query = bibleSearch_->text().trimmed();
  BiblePassage passage = bible_.search(query);
  if (passage.valid) {
    showPassage(passage);
    return;
  }

  const QString completed = bible_.completeReference(query);
  if (!completed.isEmpty() && completed.trimmed() != query) {
    bibleSearch_->setText(completed);
    bibleSearch_->setCursorPosition(completed.size());
    passage = bible_.search(completed);
    if (passage.valid) showPassage(passage);
    return;
  }

  bibleResult_->setPlainText("No se encontró. Ejemplos: deu 6:4 · sal 23 · juan 3:16");
}

Project OperatorPage::projectForBiblePassage(const BiblePassage &passage) const
{
  if (!passage.valid) return {};
  const QString id = TemplateLibrary::preferredBibleTemplate();
  Project project = id.isEmpty() ? Project{} : projectForTemplateId(id);

  if (project.layers.isEmpty() || !fillBibleFields(project, passage.text, passage.reference))
    project = TemplateFactory::scriptureLowerThird(passage.text, passage.reference);

  return project;
}

void OperatorPage::showPassage(const BiblePassage &passage)
{
  if (!passage.valid) {
    bibleResult_->setPlainText("No se encontró la referencia.");
    return;
  }

  currentPassage_ = passage;
  bibleSearch_->setText(passage.reference);
  bibleResult_->setPlainText(passage.reference + "\n\n" + passage.text);

  const Project preview = projectForBiblePassage(passage);
  if (!preview.layers.isEmpty())
    AppState::instance().loadProject(preview);
  refreshBibleTemplateLabel();
}

void OperatorPage::navigateBible(int delta)
{
  if (!currentPassage_.valid) {
    handleBibleEnter();
    return;
  }
  showPassage(bible_.adjacent(currentPassage_, delta));
}

void OperatorPage::refreshBibleTemplateLabel()
{
  const QString id = TemplateLibrary::preferredBibleTemplate();
  const QString name = TemplateLibrary::displayNameForId(id);
  bibleTemplateLabel_->setText(name.isEmpty() ? "Biblia: sin plantilla" : "Biblia: ★ " + name);
}

void OperatorPage::addBibleToService()
{
  if (!currentPassage_.valid) {
    handleBibleEnter();
    if (!currentPassage_.valid) return;
  }

  Project project = projectForBiblePassage(currentPassage_);
  if (project.layers.isEmpty()) {
    QMessageBox::information(this, "Biblia", "Crea o selecciona una plantilla bíblica en Diseño.");
    return;
  }

  appendPrepared("Versículo · " + currentPassage_.reference, "scripture", project, true);
  tabs_->setCurrentIndex(0);
}

void OperatorPage::refreshProgram()
{
  const QImage frame = AppState::instance().programFrame();
  if (frame.isNull()) {
    programScreen_->clear();
    return;
  }

  programScreen_->setPixmap(QPixmap::fromImage(frame).scaled(programScreen_->size(),
                                                                  Qt::KeepAspectRatio,
                                                                  Qt::SmoothTransformation));
}

} // namespace wg
