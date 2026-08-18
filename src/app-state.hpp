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

  void resetDemoProject();
  void rebuildPreview();
  void showPreviewOnProgram();
  void hideProgram();
  void setLayerText(int index, const QString &text);
  void notifyModelChanged();

signals:
  void previewChanged();
  void programChanged();
  void modelChanged();
  void onAirChanged(bool visible);

private:
  AppState();
  void renderProgramAnimation(qreal progress, bool entering);

  Project project_;
  mutable QReadWriteLock frameLock_;
  QImage previewFrame_;
  QImage programFrame_;
  bool programVisible_ = false;
  QVariantAnimation animation_;
};

} // namespace wg
