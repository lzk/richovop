#include "scalingsettingkeyboard.h"
#include "ui_scalingsettingkeyboard.h"

ScalingSettingKeyboard::ScalingSettingKeyboard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScalingSettingKeyboard)
{
    ui->setupUi(this);

    //限制在LineEdit輸入整數25~9999
    QValidator *Validator = new QIntValidator(25,999,this);
    ui->lineEdit_ShowScalingPercent->setValidator(Validator);
//    ui->lineEdit_ShowScalingPercent->setText(QString::number(Temp::scalingTemp,10));
}
void ScalingSettingKeyboard::set_num(int num)
{
    ui->lineEdit_ShowScalingPercent->setText(QString::number(num,10));
}

ScalingSettingKeyboard::~ScalingSettingKeyboard()
{
    delete ui;
}

void ScalingSettingKeyboard::on_btn_OK_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    if (i<25||i>400)
        {
            QMessageBox *Warning = new QMessageBox;
            Warning->setText("Please enter value between 25 to 400.");
            Warning->setWindowTitle("Warning !");
            Warning->setStandardButtons(QMessageBox::Close);
            Warning->exec();
        }
    else
    {
        emit sendScalingData(p);
//        QThread *running = new QThread(this);
//        bool isShutDown = !running->isRunning();
//        if (isShutDown)
//            close();
        close();
    }
}

void ScalingSettingKeyboard::on_btn_1_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+1;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_2_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+2;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_3_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+3;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_4_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+4;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_5_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+5;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_6_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+6;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_7_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+7;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_8_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+8;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_9_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10+9;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_0_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i*10;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_Backspace_clicked()
{
    QString p=ui->lineEdit_ShowScalingPercent->text();
    int i= p.toInt();
    i=i/10;
    ui->lineEdit_ShowScalingPercent->setText(QString::number(i,10));
}

void ScalingSettingKeyboard::on_btn_Clear_clicked()
{
    ui->lineEdit_ShowScalingPercent->clear();
}

void ScalingSettingKeyboard::on_lineEdit_ShowScalingPercent_textChanged(const QString &arg1)    //set limitation and enabled/disabled "OK" button
{
    if (ui->lineEdit_ShowScalingPercent->text().toInt()>400 || ui->lineEdit_ShowScalingPercent->text().toInt()<25)
        ui->btn_OK->setEnabled(false);
    else
        ui->btn_OK->setEnabled(true);
    if (ui->lineEdit_ShowScalingPercent->text().toInt()>400)
        ui->lineEdit_ShowScalingPercent->setText("400");
}
