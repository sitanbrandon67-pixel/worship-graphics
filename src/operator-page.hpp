#pragma once

#include "bible-engine.hpp"
#include "model.hpp"
#include <QVector>
#include <QWidget>

class QLabel;
class QListWidget;
class QLineEdit;
class QTextEdit;
class QTabWidget;
class QShowEvent;
class QCompleter;
class QStringListModel;

namespace wg {

struct PreparedGraphic {
  QString label;
  QString kind;
  Project project;
};

class OperatorPage final : public QWidget {
  Q_OBJECT
public:
  explicit OperatorPage(QWidget *parent = nullptr);

protected:
  void showEvent(QShowEvent *event) override;

private slots:
  void refreshProgram();
  void refreshServiceTemplates();
  void previewServiceTemplate(int row);
  void addSelectedTemplateToService();

  void loadPreparedSelection(int row);
  void removePreparedGraphic();
  void movePreparedUp();
  void movePreparedDown();
  void previousPrepared();
  void nextPrepared();

  void installBible();
  void handleBibleEnter();
  void addBibleToService();
  void navigateBible(int delta);
  void updateBibleSuggestions(const QString &text);

private:
  void showPassage(const BiblePassage &passage);
  void tryLoadInstalledBible();
  void acceptBibleSuggestion(const QString &bookName);
  void refreshBibleTemplateLabel();

  void seedPreparedService();
  void appendPrepared(const QString &label, const QString &kind, const Project &project, bool select = true);
  void rebuildPreparedList(int selectedRow = -1);
  Project projectForTemplateId(const QString &templateId) const;
  Project projectForBiblePassage(const BiblePassage &passage) const;

  QLabel *programScreen_ = nullptr;
  QLabel *statusLabel_ = nullptr;
  QLabel *preparedLabel_ = nullptr;
  QLabel *programNameLabel_ = nullptr;

  QListWidget *serviceTemplates_ = nullptr;
  QListWidget *serviceList_ = nullptr;
  QTabWidget *tabs_ = nullptr;

  QLineEdit *bibleSearch_ = nullptr;
  QTextEdit *bibleResult_ = nullptr;
  QLabel *bibleStatus_ = nullptr;
  QLabel *bibleTemplateLabel_ = nullptr;
  QCompleter *bibleCompleter_ = nullptr;
  QStringListModel *bibleSuggestionModel_ = nullptr;

  BibleEngine bible_;
  BiblePassage currentPassage_;
  QVector<PreparedGraphic> prepared_;
};

} // namespace wg
