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
class CopiesSettingKeyboard;
class ScalingSettingKeyboard;

namespace Ui {
class MainWidget;
class TabCopy;
class TabSetting;
class TabAbout;
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

private:
    Ui::MainWidget *ui;
    Ui::TabCopy *tc;
    Ui::TabSetting *ts;
    Ui::TabAbout *ta;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
//    QAction* action_refresh;
    QTimer timer;
    QProgressDialog* progressDialog;
    bool device_status;
    DeviceManager* device_manager;
    MessageBox msgBox;
    MessageBox msgBox_info;

    void initializeUi();
    void retranslateUi();
    void createActions();
    void updateUi();
    void initialize();

    void emit_cmd(int);
    void install_next_callback(const char *member);
signals:
   void signals_cmd(int);
   void signals_deviceChanged(const QString&);
   void signals_emit_cmd(int);
   void signals_cmd_next();
   void signals_cmd_result(int ,int);
   void signals_cmd_result_copy(int ,int);
   void signals_cmd_result_setting(int ,int);

public slots:
   void slots_cmd_result(int ,int);
   void slots_cmd();
   void slots_timeout();
   void slots_progressBar(int ,int);
   void on_refresh_clicked();
private slots:
    void on_comboBox_deviceList_activated(int index);
    void slots_cmd_complete();

private:
    void cmdResult_getDeviceStatus(int err);
    void cmdResult_passwd_confirmForApply(int err);
    void cmdResult_wifi_apply(int err);
    void cmdResult_passwd_confirmForSetPasswd(int err);
    void cmdResult_passwd_set(int err);
    void cmdResult_wifi_getAplist(int err);
    void cmdResult_wifi_get(int err);
    int cmdResult_emit_next(int err);
    void cmdResult_passwd_confirmed(int err);

private:
    ///////////////////////////tab about//////////////////////
//    QAction* action_about_update;
    void initializeTabAbout();
    void slots_about_update();

    ///////////////////////////tab copy///////////////////
    //    QAction* action_copy_default;
    QStringList stringlist_output_size;
    ScalingSettingKeyboard* keyboard_scaling;
    CopiesSettingKeyboard* keyboard_copies;
    void initializeTabCopy();
    void updateCopy();
    void copy_button_IDCardCopy();

    //////////////////////////tab setting///////////////////
    int wifi_encryptionType;
    QString wifi_ssid;
    QString wifi_password;
    int wifi_wepIndex;
    QString wifi_ms_password;
    QString wifi_sw_password;
    int wifi_ms_wepIndex;
    int wifi_sw_wepIndex;
#define NUM_OF_APLIST 10
    int wifi_sw_encryptionType[NUM_OF_APLIST];
    QString machine_wifi_ssid;
//    QString machine_wifi_password;

    static const int wifi_default_encryptionType = 2;//WPA2-PSK-AES
    static const int wifi_default_wepIndex = 0;

    bool passwd_checked;
    QString passwd;
    void initializeTabSetting();
    bool wifi_validate_ssidPassword();
    void wifi_update_encryptionType();
    void wifi_update_Data();
    void wifi_update();
    void wifi_passwd_doConfirm();
    void wifi_passwd_doConfirm(const char *member);
    void passwd_set_doConfirm();
    void result_wifi_getAplist();
    void wifi_update_checkbox(bool);
    void wifi_init();
    void wifi_apply();
    void wifi_getStatusToRefreshAplist();
    int wifi_getStatus();

private slots:
    //////////////////tab about///////////////////////////
    /////////////////tab copy///////////////////////////////
    void slots_copy_combo(int);
    void slots_copy_pushbutton();
    void slots_copy_radio(bool);
    void slots_copy_keyboard(QString);
    //////////////////////////tab setting///////////////////
    void slots_wifi_radiobutton(bool);
    void slots_wifi_textChanged(const QString &arg1);
    void slots_wifi_checkbox(bool);
    void slots_wifi_enable();
    void slots_wifi_getStatusToRefreshAplist();
    void slots_wifi_refreshAplist();
    void slots_passwd_set();
    void slots_wifi_getStatusToApply();
    void slots_passwd_comfirmed();
    void spinBox_PSaveTime_valueChanged(int arg1);
    void checkBox_powerOff_toggled(bool checked);
    void checkBox_tonerEnd_toggled(bool checked);
};

#endif // MAINWIDGET_H
