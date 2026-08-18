#pragma once
#include "model.hpp"

namespace wg {
class TemplateFactory {
public:
  static Project pastorLowerThird();
  static Project motionPiecesLowerThird();
  static Project scriptureLowerThird(const QString &verse = "Porque de tal manera amó Dios al mundo...",
                                     const QString &reference = "Juan 3:16");
  static Project sermonTitleLowerThird();
  static Project worshipLowerThird();
  static Project announcementLowerThird();
};
} // namespace wg
