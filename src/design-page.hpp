#pragma once
#include <QWidget>

class QLabel;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;

namespace wg {
class TimelineWidget;
class DesignCanvasWidget;

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
  void createBlankCanvas();
  void importPsd();
  void saveTemplate();
  void loadSelectedTemplate();
  void markBibleTemplate(bool checked);
  void markAsVerseField();
  void markAsReferenceField();
  void alignLayerLeft();
  void alignLayerCenterH();
  void alignLayerRight();
  void alignLayerTop();
  void alignLayerCenterV();
  void alignLayerBottom();

private:
  void refreshCanvas();
  void setCurrentRowSafe(int row);
  bool currentLayerIsText() const;

  QListWidget *templates_ = nullptr;
  QListWidget *layers_ = nullptr;
  DesignCanvasWidget *canvas_ = nullptr;
  QLabel *canvasInfo_ = nullptr;
  QLineEdit *name_ = nullptr;
  QLineEdit *text_ = nullptr;
  QLineEdit *color_ = nullptr;
  QSpinBox *x_ = nullptr;
  QSpinBox *y_ = nullptr;
  QSpinBox *w_ = nullptr;
  QSpinBox *h_ = nullptr;
  QSpinBox *fontSize_ = nullptr;
  QSpinBox *minFontSize_ = nullptr;
  QSpinBox *maxLines_ = nullptr;
  QSpinBox *opacity_ = nullptr;
  QSpinBox *radius_ = nullptr;
  QDoubleSpinBox *rotation_ = nullptr;
  QSpinBox *enterDelay_ = nullptr;
  QSpinBox *exitDelay_ = nullptr;
  QSpinBox *enterDuration_ = nullptr;
  QSpinBox *exitDuration_ = nullptr;
  QComboBox *enterAnimation_ = nullptr;
  QComboBox *exitAnimation_ = nullptr;
  QComboBox *textAlignH_ = nullptr;
  QComboBox *textAlignV_ = nullptr;
  QCheckBox *autoFit_ = nullptr;
  QCheckBox *wrap_ = nullptr;
  QCheckBox *splitOverflow_ = nullptr;
  QCheckBox *bibleTemplate_ = nullptr;
  TimelineWidget *timeline_ = nullptr;
  int currentRow_ = -1;
};
} // namespace wg
