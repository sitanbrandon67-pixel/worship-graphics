#pragma once

#include "bible-engine.hpp"
#include "model.hpp"
#include <QVector>
#include <QWidget>

class QLabel;
class QListWidget;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QTabWidget;

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

private slots:
  void refreshProgram();

  void loadPreparedSelection(int row);
  void addPreparedGraphic();
  void removePreparedGraphic();
  void movePreparedUp();
  void movePreparedDown();
  void previousPrepared();
  void nextPrepared();

  void installBible();
  void searchBible();
  void prepareBibleForProgram();
  void addBibleToService();
  void navigateBible(int delta);
  void refreshBibleSelectors();
  void refreshChapters();
  void refreshVerses();
  void selectBibleVerse();

private:
  void showPassage(const BiblePassage &passage);
  void tryLoadInstalledBible();

  void seedPreparedService();
  void appendPrepared(const QString &label, const QString &kind, const Project &project, bool select = true);
  void rebuildPreparedList(int selectedRow = -1);
  Project projectForServiceKind(const QString &kind) const;

  QLabel *programScreen_ = nullptr;
  QLabel *statusLabel_ = nullptr;
  QLabel *preparedLabel_ = nullptr;
  QLabel *programNameLabel_ = nullptr;

  QListWidget *serviceList_ = nullptr;
  QComboBox *serviceType_ = nullptr;
  QTabWidget *tabs_ = nullptr;

  QLineEdit *bibleSearch_ = nullptr;
  QTextEdit *bibleResult_ = nullptr;
  QLabel *bibleStatus_ = nullptr;
  QComboBox *book_ = nullptr;
  QComboBox *chapter_ = nullptr;
  QComboBox *verse_ = nullptr;

  BibleEngine bible_;
  BiblePassage currentPassage_;
  QVector<PreparedGraphic> prepared_;
};

} // namespace wg
