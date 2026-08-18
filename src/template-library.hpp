#pragma once

#include "model.hpp"
#include <QString>
#include <QVector>

namespace wg {

struct TemplateEntry {
  QString name;
  QString filePath;
  QString thumbnailPath;
};

class TemplateLibrary {
public:
  static QString libraryPath();
  static QVector<TemplateEntry> entries();
  static bool save(const Project &project, const QString &name, QString *error = nullptr);
  static bool load(const QString &filePath, Project *project, QString *error = nullptr);
};

} // namespace wg
