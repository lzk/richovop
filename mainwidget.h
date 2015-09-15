/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QMessageBox>

class QProgressDialog;
class DeviceManager;

namespace Ui {
class MainWidget;
class TabSetting;
}

class MessageBox:public QMessageBox
{
protected:
    void showEvent(QShowEvent* event)
    {
        QWidget* label = findChild<QWidget*>("qt_msgbox_label");
        if(label){
//            label->setMinimumSize(400,300);
            label->setMinimumWidth(400);
        }
        QMessageBox::showEvent(event);
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
    Ui::TabSetting *ts;

//    QAction* action_refresh;
    QTimer timer;
    QProgressDialog* progressDialog;
    DeviceManager* device_manager;
    MessageBox msgBox;
    MessageBox msgBox_info;

    void initializeUi();
    void retranslateUi();
    void createActions();
    void updateUi();
    void initialize();

    void initializeTabCopy();
    void initializeTabSetting();

public:
    QMessageBox::StandardButton messagebox_exec(const QString &text,
                                                QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                               QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
                                                const QString &title = "");
//                                               const QString &title = "<h3>" + tr("Lenovo Virtual Panel") + "</h3>");
    void messagebox_show(const QString &text,
                         QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
                        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
                         const QString &title = "");
//                        const QString &title = "<h3>" + tr("Lenovo Virtual Panel") + "</h3>");
    void messagebox_hide(){msgBox_info.hide();}

signals:
   void signals_deviceChanged(const QString&);
   void signals_cmd_result(int ,int);
   void signals_cmd_result_copy(int ,int);
   void signals_cmd_result_setting(int ,int);

public slots:
   void slots_cmd_result(int ,int);
   void slots_timeout();
   void slots_progressBar(int ,int);
   void on_refresh_clicked();

private slots:
    void on_comboBox_deviceList_activated(int index);

};

#endif // MAINWIDGET_H
