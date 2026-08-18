#include "theme.hpp"
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QWidget>

namespace wg {

void applySoftShadow(QWidget *widget)
{
  auto *shadow = new QGraphicsDropShadowEffect(widget);
  shadow->setBlurRadius(18.0);
  shadow->setOffset(0.0, 5.0);
  shadow->setColor(QColor(0, 0, 0, 90));
  widget->setGraphicsEffect(shadow);
}

QString applicationTheme()
{
  return R"QSS(
    QWidget#wgRoot {
      background: #252525;
      color: #ECECEC;
      font-family: "Segoe UI";
      font-size: 11px;
    }

    QWidget#wgTopBar {
      background: #2D2D2D;
      border-bottom: 1px solid #414141;
    }

    QLabel#wgBrand {
      font-size: 14px;
      font-weight: 750;
      color: #F3F3F3;
      letter-spacing: 0.5px;
    }

    QLabel#wgSubtle {
      color: #9B9B9B;
      font-size: 10px;
    }

    QLabel#wgSectionTitle {
      color: #D7D7D7;
      font-weight: 700;
      font-size: 10px;
      letter-spacing: 0.6px;
    }

    QFrame#wgCard {
      background: #383838;
      border: 1px solid #474747;
      border-radius: 11px;
    }

    QFrame#wgFloatingBar {
      background: #343434;
      border: 1px solid #4B4B4B;
      border-radius: 10px;
    }

    QPushButton {
      background: #444444;
      color: #E2E2E2;
      border: 1px solid #505050;
      border-radius: 7px;
      padding: 4px 7px;
      min-height: 16px;
      font-size: 10px;
    }

    QPushButton:hover {
      background: #4D4D4D;
      border-color: #616161;
    }

    QPushButton:pressed { background: #303030; }
    QPushButton:disabled { color: #777777; background: #353535; border-color: #414141; }

    QPushButton#wgIconButton {
      min-width: 24px;
      max-width: 28px;
      min-height: 24px;
      max-height: 28px;
      padding: 2px;
      border-radius: 6px;
      font-size: 11px;
      font-weight: 700;
    }

    QPushButton#wgSoftButton {
      background: #3B3B3B;
      color: #CFCFCF;
    }

    QPushButton#wgPrimary {
      background: #D5D5D5;
      color: #282828;
      border: 1px solid #E2E2E2;
      font-weight: 750;
    }

    QPushButton#wgPrimary:hover { background: #E6E6E6; }

    QPushButton#wgDanger {
      background: #4B3636;
      color: #F2DADA;
      border: 1px solid #644747;
    }

    QPushButton#wgMode {
      border-radius: 7px;
      padding: 4px 8px;
    }

    QPushButton#wgMode:checked {
      background: #555555;
      border-color: #737373;
      color: #FFFFFF;
    }

    QListWidget, QTextEdit {
      background: #303030;
      border: 1px solid #484848;
      border-radius: 8px;
      outline: 0;
      color: #E7E7E7;
      selection-background-color: #555555;
      font-size: 10px;
    }

    QListWidget::item {
      padding: 4px;
      border-radius: 5px;
    }

    QListWidget::item:selected {
      background: #505050;
      color: #FFFFFF;
    }

    QListWidget#wgPreparedList::item {
      margin: 1px 0;
      padding: 5px;
    }

    QListWidget#wgTemplateLibrary {
      background: transparent;
      border: none;
    }

    QListWidget#wgTemplateLibrary::item {
      background: #303030;
      border: 1px solid #484848;
      border-radius: 8px;
      padding: 5px;
    }

    QListWidget#wgTemplateLibrary::item:selected {
      background: #4A4A4A;
      border-color: #7A7A7A;
    }

    QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
      background: #303030;
      border: 1px solid #4A4A4A;
      border-radius: 6px;
      padding: 4px 6px;
      color: #ECECEC;
      min-height: 16px;
      font-size: 10px;
    }

    QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
      border-color: #727272;
    }

    QComboBox QAbstractItemView {
      background: #333333;
      color: #ECECEC;
      selection-background-color: #555555;
      font-size: 10px;
    }

    QLabel#wgScreen, QWidget#wgScreen {
      background: #181818;
      border: 1px solid #454545;
      border-radius: 8px;
      padding: 2px;
    }

    QTabWidget#wgOperatorTabs::pane {
      background: #333333;
      border: 1px solid #474747;
      border-radius: 8px;
      top: -1px;
    }

    QTabBar::tab {
      background: #303030;
      color: #AFAFAF;
      border: 1px solid #444444;
      border-bottom: none;
      padding: 5px 10px;
      margin-right: 2px;
      border-top-left-radius: 6px;
      border-top-right-radius: 6px;
      font-size: 10px;
    }

    QTabBar::tab:selected {
      background: #414141;
      color: #FFFFFF;
      border-color: #5A5A5A;
    }

    QWidget#wgTimeline {
      background: #303030;
      border: 1px solid #474747;
      border-radius: 10px;
    }

    QScrollBar:vertical {
      background: #2B2B2B;
      width: 9px;
      margin: 1px;
    }
    QScrollBar::handle:vertical {
      background: #565656;
      min-height: 24px;
      border-radius: 4px;
    }
    QScrollBar:horizontal {
      background: #2B2B2B;
      height: 9px;
      margin: 1px;
    }
    QScrollBar::handle:horizontal {
      background: #565656;
      min-width: 24px;
      border-radius: 4px;
    }
  )QSS";
}

} // namespace wg
