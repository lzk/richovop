#include "logo_icon.h"
#include "ui_logo_icon.h"

Logo_icon::Logo_icon(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Logo_icon)
{
    ui->setupUi(this);
}

Logo_icon::~Logo_icon()
{
    delete ui;
}
