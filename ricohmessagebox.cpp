#include "ricohmessagebox.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QAbstractButton>

#include "version.h"
QMessageBox::StandardButton RicohMessageBox::app_messagebox_exec(const QString &text,
                                                QMessageBox::StandardButtons buttons,
                                               QMessageBox::StandardButton defaultButton)
{
    QMessageBox msgbox;
    QMessageBox* mb = & msgbox;
    mb->setWindowIcon(QIcon(":/printer.png"));
    mb->setWindowTitle(vop_name);
    mb->setText(text);
    mb->setIconPixmap(QPixmap(":/printer.png"));

//    mb->setText(title);
//    mb->setInformativeText(text);

    mb->setStandardButtons(buttons);
    mb->setDefaultButton(defaultButton);
    QAbstractButton* ab = mb->button(QMessageBox::Cancel);
    if(ab){
        ab->setText(tr("IDS_Cancel"));
    }
    ab = mb->button(QMessageBox::Ok);
    if(ab){
        ab->setText(tr("IDS_OK"));
    }
//    mb->setWindowFlags(Qt::FramelessWindowHint);
#if 1
    mb->show();//show first before get real size
//    QPoint widget_pos = mapToGlobal(pos());
//    mb->move(widget_pos.x() + (width() - mb->width())/2,
//         widget_pos.y() + (height() - mb->height())/2 - 50);
    mb->move((QApplication::desktop()->width() - mb->width())/2,
          (QApplication::desktop()->height() - mb->height())/2);
//    mb->raise();
//    mb->activateWindow();
#endif
    return (QMessageBox::StandardButton)mb->exec();
}

void RicohMessageBox::messagebox_exec(const QString &text)
{
    QMessageBox msgbox;
    QMessageBox* mb = & msgbox;
    mb->setIconPixmap(QPixmap(":/printer.png"));
    mb->setText("<h3>" + QString(vop_name) + "</h3>");

//    setInformativeText(text);
    mb->setDefaultButton(QMessageBox::NoButton);
    mb->setWindowTitle(" ");
    mb->setWindowFlags(Qt::FramelessWindowHint);

    QWidget* label = mb->findChild<QWidget*>("qt_msgbox_label");
    if(label){
//            label->setMinimumSize(400,300);
        label->setMinimumWidth(400);
    }
    mb->setStandardButtons(QMessageBox::Ok);
    mb->setInformativeText(text);
#if 1
    mb->show();//show first before get real size
//    QPoint widget_pos = mapToGlobal(pos());
//    move(widget_pos.x() + (width() - width())/2,
//         widget_pos.y() + (height() - height())/2 - 50);
    mb->move((QApplication::desktop()->width() - mb->width())/2,
          (QApplication::desktop()->height() - mb->height())/2);
#endif
    mb->exec();
}

RicohMessageBox::RicohMessageBox(QWidget *parent) :
    QMessageBox(parent)
{
    setIconPixmap(QPixmap(":/printer.png"));
    setText("<h3>" + QString(vop_name) + "</h3>");

//    setInformativeText(text);
    setDefaultButton(QMessageBox::NoButton);
    setWindowTitle(" ");
    setWindowFlags(Qt::FramelessWindowHint);

    QWidget* label = findChild<QWidget*>("qt_msgbox_label");
    if(label){
//            label->setMinimumSize(400,300);
        label->setMinimumWidth(400);
    }
}

void RicohMessageBox::messagebox_show(const QString &text)
{
    if(!text.compare(informativeText()) && isVisible()){
        return;
    }
    setStandardButtons(QMessageBox::NoButton);
    setInformativeText(text);
    setModal(false);
    adjustSize();
    show();//show first before get real size

//    QPoint widget_pos = mapToGlobal(pos());
//    move(widget_pos.x() + (width() - width())/2,
//         widget_pos.y() + (height() - height())/2 - 50);
    move((QApplication::desktop()->width() - width())/2,
          (QApplication::desktop()->height() - height())/2);
}

TonerMessageBox::TonerMessageBox(QWidget *parent) :
    QMessageBox(parent)
{
    setIconPixmap(QPixmap(":/printer.png"));
    setText("<h3>" + QString(vop_name) + "</h3>");

//    setInformativeText(text);
    setStandardButtons(QMessageBox::Help| QMessageBox::Close);
    QAbstractButton* ab = button(QMessageBox::Help);
    if(ab){
        ab->setText(tr("Supply Order"));
        connect(ab ,SIGNAL(clicked()) ,this ,SLOT(button_pressed()));
    }
    ab = button(QMessageBox::Close);
    if(ab){
        ab->setText(tr("ResStr_Close"));
    }
    setDefaultButton(QMessageBox::NoButton);
    setWindowFlags(Qt::FramelessWindowHint);
    setModal(false);

    QWidget* label = findChild<QWidget*>("qt_msgbox_label");
    if(label){
//            label->setMinimumSize(400,300);
        label->setMinimumWidth(400);
    }

}

void TonerMessageBox::messagebox_show(const QString &text)
{
    if(!text.compare(informativeText()) && isVisible()){
        return;
    }
    setInformativeText(text);
    adjustSize();
    show();//show first before get real size
    move((QApplication::desktop()->width() - width())/2,
          (QApplication::desktop()->height() - height())/2);

}

#include <QDesktopServices>
#include<QUrl>
void TonerMessageBox::button_pressed()
{
    if(!QDesktopServices::openUrl(QUrl("http://www.ricoh.com/printers/sp150/supply/gateway/"))){
    }
}

//void MainWidget::supply_order(QAbstractButton* button)
//{
//    if(QMessageBox::RejectRole == msgBox_toner->buttonRole(button)){
//        if(!QDesktopServices::openUrl(QUrl("http://www.ricoh.com/printers/sp150/supply/gateway/"))){
//        }
//    }
//}
