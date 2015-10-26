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

public slots:
   void slots_cmd_result(int ,int);
   void on_listWidget_currentRowChanged(int currentRow);
private slots:
    void slots_wifi_radiobutton(bool);
    void slots_wifi_textChanged(const QString &arg1);

    void on_spinBox_PSaveTime_valueChanged(int arg1);
    void on_checkBox_powerOff_toggled(bool checked);
    void on_checkBox_tonerEnd_toggled(bool checked);
    void on_btn_apply_ws_clicked();
    void on_btn_apply_mp_clicked();
    void on_btn_refresh_clicked();

    void on_ipa_rb_ipv4_toggled(bool checked);

    void on_ip_btn_apply_clicked();


    void on_le_newPassword_textEdited(const QString &arg1);

    void on_le_confirmPassword_textEdited(const QString &arg1);

public:
    Ui::TabSetting *ui;

private:
    MainWidget* main_widget;
    DeviceManager* device_manager;

    QString last_newPassword;
    QString last_confirmPassword;
    QString wifi_ssid;
    QString wifi_password;
    QString wifi_ms_password;
    QString wifi_sw_password;

#define NUM_OF_APLIST 10
    int wifi_aplist_encryptionType[NUM_OF_APLIST];
    int wifi_encryptionType;
    int wifi_wepIndex;

    int wifi_ms_encryptionType;
    int wifi_ms_wepIndex;

    int wifi_sw_encryptionType;
    int wifi_sw_wepIndex;

    static const int wifi_default_encryptionType = 2;//WPA2-PSK-AES
    static const int wifi_default_wepIndex = 0;

private:
    bool disable_emit;

private:
    void init_wifi();
    void init_ip();

    bool wifi_validate_ssidPassword();
    bool wifi_validate_ssidPassword(QString,QString,int);
    void wifi_update_encryptionType();
    void wifi_update_encryptionType(int);
    void wifi_update_Data();
    void wifi_update();
    void wifi_update(QString,QString,int);
    void result_wifi_getAplist();
    bool setting_confirmPasswd();
};


#endif // TABSETTING_H
