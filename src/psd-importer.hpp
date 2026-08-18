#pragma once

#include "model.hpp"

#include <QString>
#include <QStringList>

namespace wg {

struct PsdImportResult {
  bool ok = false;
  Project project;
  int textLayers = 0;
  int imageLayers = 0;
  int groupLayers = 0;
  int skippedLayers = 0;
  QStringList warnings;
  QString error;
};

class PsdImporter {
public:
  static PsdImportResult importFile(const QString &filePath);
};

} // namespace wg
