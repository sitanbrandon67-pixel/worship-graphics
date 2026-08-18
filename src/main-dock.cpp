#include "main-dock.hpp"
#include "design-page.hpp"
#include "operator-page.hpp"
#include "theme.hpp"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace wg {

MainDock::MainDock(QWidget *parent) : QWidget(parent)
{
  setObjectName("wgRoot");
  setStyleSheet(applicationTheme());

  // Operator can now live comfortably as a narrow OBS dock.
  setMinimumSize(470, 520);

  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0);
  root->setSpacing(0);

  auto *top = new QWidget();
  top->setObjectName("wgTopBar");

  auto *topLayout = new QHBoxLayout(top);
  topLayout->setContentsMargins(14, 9, 14, 9);
  topLayout->setSpacing(8);

  auto *brand = new QLabel("WORSHIP GRAPHICS");
  brand->setObjectName("wgBrand");

  auto *version = new QLabel("v" WG_VERSION);
  version->setObjectName("wgSubtle");

  auto *operatorMode = new QPushButton("OPERADOR");
  auto *designMode = new QPushButton("DISEÑO");

  operatorMode->setObjectName("wgMode");
  designMode->setObjectName("wgMode");
  operatorMode->setCheckable(true);
  designMode->setCheckable(true);
  operatorMode->setChecked(true);

  auto *modes = new QButtonGroup(this);
  modes->setExclusive(true);
  modes->addButton(operatorMode, 0);
  modes->addButton(designMode, 1);

  topLayout->addWidget(brand);
  topLayout->addWidget(version);
  topLayout->addStretch();
  topLayout->addWidget(operatorMode);
  topLayout->addWidget(designMode);

  root->addWidget(top);

  pages_ = new QStackedWidget();
  pages_->addWidget(new OperatorPage());
  pages_->addWidget(new DesignPage());
  root->addWidget(pages_, 1);

  connect(modes, &QButtonGroup::idClicked, this, [this](int id) {
    pages_->setCurrentIndex(id);

    if (id == 0) {
      // Compact operator workspace.
      setMinimumSize(470, 520);
    } else {
      // Design keeps the full editor workspace.
      setMinimumSize(1120, 720);
    }
  });
}

} // namespace wg
