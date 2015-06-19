#ifndef SCALINGSETTINGKEYBOARD_H
#define SCALINGSETTINGKEYBOARD_H

#include <QDialog>
#include <QThread>
#include <QMessageBox>
//#include "temp.h"


namespace Ui {
class ScalingSettingKeyboard;
}

class ScalingSettingKeyboard : public QDialog
{
    Q_OBJECT

public:
    explicit ScalingSettingKeyboard(QWidget *parent = 0);
    ~ScalingSettingKeyboard();

    void set_num(int);
private slots:
    void on_btn_OK_clicked();

    void on_btn_1_clicked();

    void on_btn_2_clicked();

    void on_btn_3_clicked();

    void on_btn_4_clicked();

    void on_btn_5_clicked();

    void on_btn_6_clicked();

    void on_btn_7_clicked();

    void on_btn_8_clicked();

    void on_btn_9_clicked();

    void on_btn_0_clicked();

    void on_btn_Backspace_clicked();

    void on_btn_Clear_clicked();

    void on_lineEdit_ShowScalingPercent_textChanged(const QString &arg1);

private:
    Ui::ScalingSettingKeyboard *ui;

signals:
    void sendScalingData(QString);
};

#endif // SCALINGSETTINGKEYBOARD_H
