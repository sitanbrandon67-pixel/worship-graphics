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

  void loadProject(const Project &project);
  void resetDemoProject();
  void loadMotionTemplate();
  void loadScriptureTemplate(const QString &verse, const QString &reference);
  void applyBiblePassage(const QString &verse, const QString &reference);

  void rebuildPreview();
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
  void staggerLayers(int stepMs = 80);

signals:
  void previewChanged();
  void programChanged();
  void modelChanged();
  void onAirChanged(bool visible);

private:
  AppState();
  void renderProgramAnimation(qreal transitionProgress, bool entering);
  int transitionDuration(bool entering) const;

  Project project_;
  mutable QReadWriteLock frameLock_;
  QImage previewFrame_;
  QImage programFrame_;
  bool programVisible_ = false;
  QVariantAnimation animation_;
};

} // namespace wg
