#include "theme.hpp"
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QWidget>

namespace wg {

void applySoftShadow(QWidget *widget)
{
  auto *shadow = new QGraphicsDropShadowEffect(widget);
  shadow->setBlurRadius(28.0);
  shadow->setOffset(0.0, 8.0);
  shadow->setColor(QColor(0, 0, 0, 105));
  widget->setGraphicsEffect(shadow);
}

QString applicationTheme()
{
  return R"QSS(
    QWidget#wgRoot {
      background: #252525;
      color: #ECECEC;
      font-family: "Segoe UI";
      font-size: 13px;
    }

    QWidget#wgTopBar {
      background: #2D2D2D;
      border-bottom: 1px solid #414141;
    }

    QLabel#wgBrand {
      font-size: 16px;
      font-weight: 750;
      color: #F3F3F3;
      letter-spacing: 0.8px;
    }

    QLabel#wgSubtle { color: #9B9B9B; }

    QLabel#wgSectionTitle {
      color: #D7D7D7;
      font-weight: 700;
      font-size: 12px;
      letter-spacing: 0.8px;
    }

    QFrame#wgCard {
      background: #383838;
      border: 1px solid #474747;
      border-radius: 18px;
    }

    QFrame#wgFloatingBar {
      background: #343434;
      border: 1px solid #4B4B4B;
      border-radius: 16px;
    }

    QPushButton {
      background: #444444;
      color: #E2E2E2;
      border: 1px solid #505050;
      border-radius: 11px;
      padding: 7px 11px;
      min-height: 18px;
    }

    QPushButton:hover {
      background: #4D4D4D;
      border-color: #5A5A5A;
    }

    QPushButton:pressed { background: #303030; }

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
      border-radius: 12px;
      padding: 6px 12px;
    }

    QPushButton#wgMode:checked {
      background: #555555;
      border-color: #737373;
      color: #FFFFFF;
    }

    QListWidget, QTextEdit {
      background: #303030;
      border: 1px solid #484848;
      border-radius: 13px;
      outline: 0;
      color: #E7E7E7;
      selection-background-color: #555555;
    }

    QListWidget::item {
      padding: 7px;
      border-radius: 8px;
    }

    QListWidget::item:selected {
      background: #505050;
      color: #FFFFFF;
    }

    QListWidget#wgPreparedList::item {
      margin: 2px 0;
      padding: 8px;
    }

    QListWidget#wgTemplateLibrary {
      background: transparent;
      border: none;
    }

    QListWidget#wgTemplateLibrary::item {
      background: #303030;
      border: 1px solid #484848;
      border-radius: 14px;
      padding: 7px;
    }

    QListWidget#wgTemplateLibrary::item:selected {
      background: #4A4A4A;
      border-color: #707070;
    }

    QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
      background: #303030;
      border: 1px solid #4A4A4A;
      border-radius: 9px;
      padding: 6px 8px;
      color: #ECECEC;
      min-height: 18px;
    }

    QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
      border-color: #727272;
    }

    QComboBox QAbstractItemView {
      background: #333333;
      color: #ECECEC;
      selection-background-color: #555555;
    }

    QLabel#wgScreen {
      background: #181818;
      border: 1px solid #454545;
      border-radius: 15px;
      padding: 3px;
    }

    QTabWidget#wgOperatorTabs::pane {
      background: #333333;
      border: 1px solid #474747;
      border-radius: 14px;
      top: -1px;
    }

    QTabBar::tab {
      background: #303030;
      color: #AFAFAF;
      border: 1px solid #444444;
      border-bottom: none;
      padding: 7px 16px;
      margin-right: 3px;
      border-top-left-radius: 9px;
      border-top-right-radius: 9px;
    }

    QTabBar::tab:selected {
      background: #414141;
      color: #FFFFFF;
      border-color: #5A5A5A;
    }

    QWidget#wgTimeline {
      background: #303030;
      border: 1px solid #474747;
      border-radius: 18px;
    }
  )QSS";
}

} // namespace wg
