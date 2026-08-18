#pragma once

#include "model.hpp"
#include <QImage>
#include <QObject>
#include <QReadWriteLock>
#include <QVariantAnimation>

namespace wg {

class AppState final : public QObject {
  Q_OBJECT
public:
  static AppState &instance();

  const Project &project() const { return project_; }
  Project &mutableProject() { return project_; }

  QImage previewFrame() const;
  QImage programFrame() const;
  bool programVisible() const { return programVisible_; }
  int timelineDuration(bool entering) const;

  void loadProject(const Project &project);
  void resetDemoProject();
  void loadMotionTemplate();
  void loadScriptureTemplate(const QString &verse, const QString &reference);
  void applyBiblePassage(const QString &verse, const QString &reference);

  void rebuildPreview();
  void renderPreviewAtTime(int elapsedMs, bool entering);
  void showPreviewOnProgram();
  void hideProgram();
  void setLayerText(int index, const QString &text);
  void notifyModelChanged();

  bool removeLayer(int index);
  bool duplicateLayer(int index);
  bool moveLayer(int index, int delta);
  bool toggleLayerVisible(int index);
  bool toggleLayerLocked(int index);
  bool groupLayers(const QVector<int> &rows);
  bool ungroupLayer(int index);
  bool setLayerTiming(int index, int delayMs, int durationMs, bool entering);
  void scaleTimeline(qreal factor, bool entering);
  void staggerLayers(int stepMs = 80);

signals:
  void previewChanged();
  void programChanged();
  void modelChanged();
  void timelineChanged();
  void onAirChanged(bool visible);

private:
  AppState();
  void renderProgramAnimation(qreal transitionProgress, bool entering);
  int transitionDuration(bool entering) const;
  int transitionDuration(const Project &project, bool entering) const;

  // project_ is the graphic currently being prepared/edited.
  Project project_;

  // programProject_ is a snapshot of the graphic that is actually on-air.
  // This keeps PREPARED and PROGRAM independent.
  Project programProject_;

  mutable QReadWriteLock frameLock_;
  QImage previewFrame_;
  QImage programFrame_;
  bool programVisible_ = false;
  QVariantAnimation animation_;
};

} // namespace wg
