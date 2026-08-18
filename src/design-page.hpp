#pragma once
#include <QWidget>

class QLabel;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;

namespace wg {
class TimelineWidget;
class DesignPage final : public QWidget {
  Q_OBJECT
public:
  explicit DesignPage(QWidget *parent = nullptr);
private slots:
  void rebuildLayerList();
  void refreshTemplateLibrary();
  void selectLayer(int row);
  void applyProperties();
  void addTextLayer();
  void addShapeLayer();
  void addImageLayer();
  void deleteCurrentLayer();
  void duplicateCurrentLayer();
  void moveCurrentLayer(int delta);
  void toggleVisibility();
  void toggleLock();
  void groupSelection();
  void ungroupCurrent();
  void applyStagger();
  void importPsdPlaceholder();
  void saveTemplate();
  void loadSelectedTemplate();
private:
  void refreshCanvas();
  void setCurrentRowSafe(int row);

  QListWidget *templates_ = nullptr;
  QListWidget *layers_ = nullptr;
  QLabel *canvas_ = nullptr;
  QLineEdit *name_ = nullptr;
  QLineEdit *text_ = nullptr;
  QLineEdit *color_ = nullptr;
  QSpinBox *x_ = nullptr;
  QSpinBox *y_ = nullptr;
  QSpinBox *w_ = nullptr;
  QSpinBox *h_ = nullptr;
  QSpinBox *fontSize_ = nullptr;
  QSpinBox *opacity_ = nullptr;
  QSpinBox *radius_ = nullptr;
  QDoubleSpinBox *rotation_ = nullptr;
  QSpinBox *enterDelay_ = nullptr;
  QSpinBox *exitDelay_ = nullptr;
  QSpinBox *duration_ = nullptr;
  QComboBox *enterAnimation_ = nullptr;
  QComboBox *exitAnimation_ = nullptr;
  TimelineWidget *timeline_ = nullptr;
  int currentRow_ = -1;
};
} // namespace wg
