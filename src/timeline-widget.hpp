#pragma once

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
#include <QVector>

class QMouseEvent;

namespace wg {

class TimelineWidget final : public QWidget {
  Q_OBJECT
public:
  explicit TimelineWidget(QWidget *parent = nullptr);
  QSize minimumSizeHint() const override { return {700, 240}; }

  bool isEntering() const { return entering_; }
  int currentTimeMs() const { return currentTimeMs_; }
  void setSelectedLayer(int layerIndex);

public slots:
  void setPhase(bool entering);
  void setCurrentTimeMs(int timeMs);
  void setPlaybackSpeed(qreal speed);
  void togglePlayback();
  void stopPlayback();
  void refreshCurrentFrame();

signals:
  void currentTimeChanged(int timeMs);
  void playbackStateChanged(bool playing);
  void layerSelected(int layerIndex);
  void timingEdited(int layerIndex);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  enum class DragMode { None, Playhead, StartHandle, EndHandle, Bar };

  QVector<int> visibleLayerIndices() const;
  int totalDurationMs() const;
  int labelWidth() const;
  QRect timelineArea() const;
  int timeToX(int ms) const;
  int xToTime(qreal x) const;
  int rowAt(qreal y) const;
  void tickPlayback();
  void applyDrag(qreal x);

  bool entering_ = true;
  int currentTimeMs_ = 0;
  int selectedLayer_ = -1;
  qreal playbackSpeed_ = 1.0;
  bool playing_ = false;

  DragMode dragMode_ = DragMode::None;
  int dragLayer_ = -1;
  int dragStartMouseMs_ = 0;
  int dragOriginDelayMs_ = 0;
  int dragOriginDurationMs_ = 450;

  QTimer playbackTimer_;
  QElapsedTimer playbackClock_;
};

} // namespace wg
