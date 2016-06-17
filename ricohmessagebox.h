#ifndef RICOHMESSAGEBOX_H
#define RICOHMESSAGEBOX_H

#include <QMessageBox>
class RicohMessageBox:public QMessageBox
{
    Q_OBJECT
public:
    explicit RicohMessageBox(QWidget *parent = 0);
//    QMessageBox::StandardButton messagebox_exec(const QString &text);
    void messagebox_show(const QString &text);

public:
    static void messagebox_exec(const QString &text);
    static QMessageBox::StandardButton app_messagebox_exec(const QString &text,
                                                QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                               QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
};

//class QAbstractButton;
class TonerMessageBox:public QMessageBox
{
    Q_OBJECT
public:
    explicit TonerMessageBox(QWidget *parent = 0);
    void messagebox_show(const QString &text);

private slots:
    void button_pressed();
//    void supply_order(QAbstractButton* button);
};
#endif // RICOHMESSAGEBOX_H
