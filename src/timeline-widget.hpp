#pragma once
#include <QWidget>

namespace wg {
class TimelineWidget final : public QWidget {
  Q_OBJECT
public:
  explicit TimelineWidget(QWidget *parent = nullptr);
  QSize minimumSizeHint() const override { return {600, 150}; }
protected:
  void paintEvent(QPaintEvent *event) override;
};
} // namespace wg
