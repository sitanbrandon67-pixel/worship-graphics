#pragma once

#include "model.hpp"
#include <QString>
#include <QVector>

namespace wg {

struct TemplateEntry {
  QString name;
  QString filePath;
  QString thumbnailPath;
  TemplateUsage usage = TemplateUsage::Generic;
};

class TemplateLibrary {
public:
  static QString libraryPath();
  static QVector<TemplateEntry> entries();
  static bool save(const Project &project, const QString &name, QString *error = nullptr);
  static bool load(const QString &filePath, Project *project, QString *error = nullptr);
  static bool remove(const QString &filePath, QString *error = nullptr);
  static bool removeOrHide(const QString &templateId, QString *error = nullptr);
  static bool isBuiltinHidden(const QString &templateId);
  static void restoreBuiltins();
  static QString displayNameForId(const QString &templateId);
  static bool isValidTemplateId(const QString &templateId);
  static bool isBibleTemplateId(const QString &templateId);
  static void setBibleDefaultTemplate(const QString &templateId);
  static QString preferredBibleTemplate();

  // templateId can be a built-in id (builtin:pastor, builtin:scripture, ...)
  // or the absolute .wgtpl path of a user template.
  static void setDefaultTemplate(const QString &serviceKind, const QString &templateId);
  static QString defaultTemplate(const QString &serviceKind);
};

} // namespace wg
