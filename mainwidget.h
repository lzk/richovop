/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTimer>
#include "version.h"
#include "ricohmessagebox.h"

class QProgressDialog;
class DeviceManager;
class TabSetting;
class TabCopy;
class TabAbout;
class QListWidget;
namespace Ui {
class MainWidget;
}
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
    TonerMessageBox* msgBox_toner;
    RicohMessageBox* msgBox_info;
//    MessageBox msgBox;
//    MessageBox msgBox_info;
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
    void messagebox_exec(const QString &text);
    void messagebox_show(const QString &text);
    void messagebox_hide();

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
    void on_button_logo_clicked();
};

#endif // MAINWIDGET_H
