#pragma once
#include "bible-engine.hpp"
#include <QWidget>

class QLabel;
class QListWidget;
class QLineEdit;
class QTextEdit;
class QComboBox;

namespace wg {
class OperatorPage final : public QWidget {
  Q_OBJECT
public:
  explicit OperatorPage(QWidget *parent = nullptr);
private slots:
  void refreshPreview();
  void refreshProgram();
  void cycleDemo(int direction);
  void installBible();
  void searchBible();
  void useBibleOnPreview();
  void navigateBible(int delta);
  void refreshBibleSelectors();
  void refreshChapters();
  void refreshVerses();
  void selectBibleVerse();
private:
  void showPassage(const BiblePassage &passage);
  void tryLoadInstalledBible();

  QLabel *previewScreen_ = nullptr;
  QLabel *programScreen_ = nullptr;
  QLabel *statusLabel_ = nullptr;
  QListWidget *serviceList_ = nullptr;
  QLineEdit *bibleSearch_ = nullptr;
  QTextEdit *bibleResult_ = nullptr;
  QLabel *bibleStatus_ = nullptr;
  QComboBox *book_ = nullptr;
  QComboBox *chapter_ = nullptr;
  QComboBox *verse_ = nullptr;
  BibleEngine bible_;
  BiblePassage currentPassage_;
  int demoIndex_ = 0;
};
} // namespace wg
