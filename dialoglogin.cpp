/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "dialoglogin.h"
#include "ui_dialoglogin.h"

QString DialogLogin::passwd = QString();

DialogLogin::DialogLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogin)
{
    ui->setupUi(this);
    Qt::WindowFlags wf = windowFlags();
    wf &= ~Qt::WindowMinMaxButtonsHint;
    setWindowFlags(wf);
    ui->pushButton->setEnabled(false);
    passwd.clear();
}

DialogLogin::~DialogLogin()
{
    delete ui;
}

void DialogLogin::on_pushButton_clicked()
{
    passwd = ui->lineEdit->text();
    accept();
}

void DialogLogin::on_lineEdit_textChanged(const QString &arg1)
{
    if(arg1.isEmpty()){
        ui->pushButton->setEnabled(false);
    }else{
        ui->pushButton->setEnabled(true);
    }
}

QString DialogLogin::getPasswd(QWidget* parent ,bool *ok)
{
    DialogLogin dialog(parent);
    int dc = dialog.exec();
    if(Accepted == dc)
        *ok = true;
    else
        *ok = false;
    return passwd;
}
