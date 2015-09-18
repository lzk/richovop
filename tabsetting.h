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
private slots:
    void slots_wifi_radiobutton(bool);
    void slots_wifi_textChanged(const QString &arg1);
//    void slots_wifi_checkbox(bool);
//    void slots_wifi_enable();

    void on_spinBox_PSaveTime_valueChanged(int arg1);
    void on_checkBox_powerOff_toggled(bool checked);
    void on_checkBox_tonerEnd_toggled(bool checked);
    void on_btn_apply_ws_clicked();
    void on_btn_apply_mp_clicked();
    void on_btn_refresh_clicked();

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
    QString passwd;

    static const int wifi_default_encryptionType = 2;//WPA2-PSK-AES
    static const int wifi_default_wepIndex = 0;

private:
    bool disable_emit;

private:
    bool wifi_validate_ssidPassword();
    void wifi_update_encryptionType();
    void wifi_update_Data();
    void wifi_update();
    void result_wifi_getAplist();
    void wifi_update_checkbox(bool);
    bool setting_confirmPasswd();
};


#endif // TABSETTING_H
