#pragma once
#include <QWidget>
class QStackedWidget;
namespace wg {
class MainDock final : public QWidget {
  Q_OBJECT
public:
  explicit MainDock(QWidget *parent = nullptr);
private:
  QStackedWidget *pages_ = nullptr;
};
}
