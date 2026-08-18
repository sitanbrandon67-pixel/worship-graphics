#include "theme.hpp"
namespace wg {
QString applicationTheme()
{
  return R"QSS(
    QWidget#wgRoot { background: #0B0F16; color: #EAF0F6; font-family: "Segoe UI"; }
    QWidget#wgTopBar { background: #101722; border-bottom: 1px solid #202A38; }
    QLabel#wgBrand { font-size: 18px; font-weight: 700; color: #FFFFFF; }
    QLabel#wgSubtle { color: #7F8DA3; }
    QPushButton { background: #182231; color: #DCE6F2; border: 1px solid #253246; border-radius: 9px; padding: 9px 14px; }
    QPushButton:hover { background: #202D40; }
    QPushButton:pressed { background: #111A26; }
    QPushButton#wgPrimary { background: #D9B35F; color: #11151C; border: none; font-weight: 700; }
    QPushButton#wgDanger { background: #4A2026; color: #FFDDE2; border: 1px solid #71323C; }
    QPushButton#wgMode:checked { background: #26364A; border-color: #D9B35F; color: #FFFFFF; }
    QFrame#wgCard { background: #111924; border: 1px solid #202B3A; border-radius: 14px; }
    QListWidget, QTreeWidget { background: #0F1620; border: 1px solid #202B3A; border-radius: 10px; outline: 0; }
    QListWidget::item, QTreeWidget::item { padding: 8px; }
    QListWidget::item:selected, QTreeWidget::item:selected { background: #26364A; color: #FFFFFF; }
    QLineEdit, QSpinBox, QComboBox { background: #0D141E; border: 1px solid #253246; border-radius: 8px; padding: 8px; color: #EAF0F6; }
    QLabel#wgScreen { background: #05070A; border: 1px solid #202B3A; border-radius: 10px; }
  )QSS";
}
}
