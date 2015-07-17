#ifndef COPIESSETTINGKEYBOARD_H
#define COPIESSETTINGKEYBOARD_H

#include <QDialog>
#include <QThread>
#include <QMessageBox>
//#include "temp.h"

namespace Ui {
class CopiesSettingKeyboard;
}

class CopiesSettingKeyboard : public QDialog
{
    Q_OBJECT

public:
    explicit CopiesSettingKeyboard(QWidget *parent = 0);
    ~CopiesSettingKeyboard();

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

    void on_lineEdit_ShowCopiesPercent_textChanged(const QString &arg1);

private:
    Ui::CopiesSettingKeyboard *ui;

signals:
    void sendCopiesData(QString);

};

#endif // COPIESSETTINGKEYBOARD_H
