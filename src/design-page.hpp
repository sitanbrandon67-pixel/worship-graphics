#pragma once
#include <QWidget>

class QLabel;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;

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
  void applyStagger();
  void importPsdPlaceholder();
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
  void setLayerName(const QString &name);
  bool hasCurrentTextLayer() const;

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
  QSpinBox *minFontSize_ = nullptr;
  QSpinBox *maxLines_ = nullptr;
  QSpinBox *opacity_ = nullptr;
  QSpinBox *radius_ = nullptr;
  QDoubleSpinBox *rotation_ = nullptr;
  QSpinBox *enterDelay_ = nullptr;
  QSpinBox *exitDelay_ = nullptr;
  QSpinBox *duration_ = nullptr;
  QComboBox *enterAnimation_ = nullptr;
  QComboBox *exitAnimation_ = nullptr;
  QComboBox *textAlignH_ = nullptr;
  QComboBox *textAlignV_ = nullptr;
  QCheckBox *autoFit_ = nullptr;
  QCheckBox *wrap_ = nullptr;
  QCheckBox *splitOverflow_ = nullptr;
  QCheckBox *bibleTemplate_ = nullptr;
  int currentRow_ = -1;
};
} // namespace wg
