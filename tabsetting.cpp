#include "tabsetting.h"
#include "ui_tabsetting.h"

TabSetting::TabSetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabSetting)
{
    ui->setupUi(this);
}

TabSetting::~TabSetting()
{
    delete ui;
}
