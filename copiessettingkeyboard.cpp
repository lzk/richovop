#include "copiessettingkeyboard.h"
#include "ui_copiessettingkeyboard.h"

CopiesSettingKeyboard::CopiesSettingKeyboard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CopiesSettingKeyboard)
{
    ui->setupUi(this);

    //限制在LineEdit輸入整數1~999
    QValidator *Validator = new QIntValidator(1,99,this);
    ui->lineEdit_ShowCopiesPercent->setValidator(Validator);
//    ui->lineEdit_ShowCopiesPercent->setText(QString::number(Temp::copiesTemp,10));
}
void CopiesSettingKeyboard::set_num(int num)
{
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(num,10));
}

CopiesSettingKeyboard::~CopiesSettingKeyboard()
{
    delete ui;
}

void CopiesSettingKeyboard::on_btn_OK_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    if (i<1||i>99)
        {
            QMessageBox *Warning = new QMessageBox;
            Warning->setText("Please enter value between 1 to 99.");
            Warning->setWindowTitle("Warning !");
            Warning->setStandardButtons(QMessageBox::Close);
            Warning->exec();
        }
    else
    {

        emit sendCopiesData(p);
//        QThread *running = new QThread(this);
//        bool isShutDown = !running->isRunning();
//        if (isShutDown)
//            close();
        close();
    }
}

void CopiesSettingKeyboard::on_btn_1_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+1;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_2_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+2;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_3_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+3;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_4_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+4;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_5_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+5;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_6_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+6;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_7_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+7;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_8_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+8;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_9_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10+9;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_0_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i*10;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_Backspace_clicked()
{
    QString p=ui->lineEdit_ShowCopiesPercent->text();
    int i= p.toInt();
    i=i/10;
    ui->lineEdit_ShowCopiesPercent->setText(QString::number(i,10));
}

void CopiesSettingKeyboard::on_btn_Clear_clicked()
{
    ui->lineEdit_ShowCopiesPercent->clear();
}

void CopiesSettingKeyboard::on_lineEdit_ShowCopiesPercent_textChanged(const QString &arg1)      //set limitation and enabled/disabled "OK" button
{
    if (ui->lineEdit_ShowCopiesPercent->text().toInt()>99 || ui->lineEdit_ShowCopiesPercent->text().toInt()<1)
        ui->btn_OK->setEnabled(false);
    else
        ui->btn_OK->setEnabled(true);
    if (ui->lineEdit_ShowCopiesPercent->text().toInt()>99)
        ui->lineEdit_ShowCopiesPercent->setText("99");
}
