/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QMessageBox>
#include "version.h"

class QProgressDialog;
class DeviceManager;
class TabSetting;
class TabCopy;
class TabAbout;
class QListWidget;
namespace Ui {
class MainWidget;
}

class MessageBox:public QMessageBox
{
public:
    MessageBox(){
        QWidget* label = findChild<QWidget*>("qt_msgbox_label");
        if(label){
//            label->setMinimumSize(400,300);
            label->setMinimumWidth(400);
        }
    }
};

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::MainWidget *ui;

    DeviceManager* device_manager;
    QListWidget *listWidget;
    TabCopy* tab_copy;
    TabSetting* tab_setting;
    TabAbout* tab_about;
//    QAction* action_refresh;
    QProgressDialog* progressDialog;
    MessageBox msgBox;
    MessageBox msgBox_info;
    QTimer timer;
    int model;
    bool no_space;
    int donot_cmd_times;

    void initializeUi();
    void retranslateUi();
    void createActions();
    void updateUi();
    void initialize();

public:
    QMessageBox::StandardButton messagebox_exec(const QString &text,
                                                QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                               QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
                                                QString title = "<h3>" + QString(vop_name) + "</h3>");
    void messagebox_show(const QString &text,
                         QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
                        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
                         QString title = "<h3>" + QString(vop_name) + "</h3>");
    void messagebox_hide(){if(msgBox_info.isVisible())msgBox_info.hide();}

signals:
   void signals_deviceChanged(const QString&);
   void signals_cmd_result(int ,int);

public slots:
   void slots_cmd_result(int ,int);
   void slots_timeout();
   void slots_progressBar(int ,int);
   void on_refresh_clicked();

private slots:
    void on_comboBox_deviceList_activated(int index);

    void on_tabWidget_currentChanged(int index);
};

#endif // MAINWIDGET_H
