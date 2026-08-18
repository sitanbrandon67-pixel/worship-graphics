#pragma once
#include <QWidget>

class QLabel;
class QListWidget;
class QLineEdit;

namespace wg {
class OperatorPage final : public QWidget {
  Q_OBJECT
public:
  explicit OperatorPage(QWidget *parent = nullptr);
private slots:
  void refreshPreview();
  void refreshProgram();
  void cycleDemo(int direction);
private:
  QLabel *previewScreen_ = nullptr;
  QLabel *programScreen_ = nullptr;
  QLabel *statusLabel_ = nullptr;
  QListWidget *serviceList_ = nullptr;
  QLineEdit *bibleSearch_ = nullptr;
  int demoIndex_ = 0;
};
}
