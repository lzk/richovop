#ifndef TABSETTING_H
#define TABSETTING_H

#include <QWidget>

namespace Ui {
class TabSetting;
}

class MainWidget;
class DeviceManager;
class TabSetting : public QWidget
{
    Q_OBJECT

public:
    explicit TabSetting(MainWidget* widget,DeviceManager* dm ,QWidget *parent = 0);
    ~TabSetting();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void hideEvent(QHideEvent *);
signals:
    void signals_cmd_next();

private slots:
    void slots_cmd();
    void slots_wifi_radiobutton(bool);
    void slots_wifi_textChanged(const QString &arg1);
    void slots_wifi_checkbox(bool);
    void slots_wifi_enable();
    void slots_wifi_getStatusToRefreshAplist();
    void slots_wifi_refreshAplist();
    void slots_passwd_set();
    void slots_wifi_getStatusToApply();
    void slots_passwd_comfirmed();

    void on_spinBox_PSaveTime_valueChanged(int arg1);
    void on_checkBox_powerOff_toggled(bool checked);
    void on_checkBox_tonerEnd_toggled(bool checked);

public:
    Ui::TabSetting *ui;

private:
    MainWidget* main_widget;
    DeviceManager* device_manager;

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

    void install_next_callback(const char *member);

public slots:
   void slots_cmd_result(int ,int);
    void slots_cmd_complete();

private:
    void cmdResult_passwd_confirmForApply(int err);
    void cmdResult_wifi_apply(int err);
    void cmdResult_passwd_confirmForSetPasswd(int err);
    void cmdResult_passwd_set(int err);
    void cmdResult_wifi_getAplist(int err);
    void cmdResult_wifi_get(int err);
    int cmdResult_emit_next(int err);
    void cmdResult_passwd_confirmed(int err);
};

#endif // TABSETTING_H
