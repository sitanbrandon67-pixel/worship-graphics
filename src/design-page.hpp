#pragma once
#include <QWidget>

class QLabel;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QComboBox;

namespace wg {
class DesignPage final : public QWidget {
  Q_OBJECT
public:
  explicit DesignPage(QWidget *parent = nullptr);
private slots:
  void rebuildLayerList();
  void selectLayer(int row);
  void applyProperties();
  void addTextLayer();
  void addShapeLayer();
  void importPsdPlaceholder();
private:
  void refreshCanvas();
  QListWidget *layers_ = nullptr;
  QLabel *canvas_ = nullptr;
  QLineEdit *text_ = nullptr;
  QSpinBox *x_ = nullptr;
  QSpinBox *y_ = nullptr;
  QSpinBox *w_ = nullptr;
  QSpinBox *h_ = nullptr;
  QSpinBox *fontSize_ = nullptr;
  QComboBox *enterAnimation_ = nullptr;
  int currentRow_ = -1;
};
}
