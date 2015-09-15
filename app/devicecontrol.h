/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef DEVICECONTROL_H
#define DEVICECONTROL_H

#include <QObject>
#include "vop_protocol.h"
class DeviceManager;
class VopDevice;
class DeviceContrl : public QObject
{
    Q_OBJECT
public:
    explicit DeviceContrl(DeviceManager* dm);
    ~DeviceContrl();

    static QString current_devicename;
    static int device_writeThenRead(char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize);
    static int device_getDeviceStatus(char* buffer ,int buffer_size);

private:
    DeviceManager* device_manager;
    VopDevice* device;
    VopProtocol* protocol;
    bool confirmed;
    int cmd_setting_confirm();
    int cmd_wifi_status();

public:
    enum{
        CMD_GetCopy,
        CMD_COPY,
        CMD_WIFI_apply,
        CMD_WIFI_get,
        CMD_WIFI_getAplist,
        CMD_PASSWD_set,
        CMD_PASSWD_get,
        CMD_PASSWD_confirm,
        CMD_WIFI_GetWifiStatus,
        CMD_PRN_TonerEnd_Get,
        CMD_PRN_TonerEnd_Set,
        CMD_PRN_PSaveTime_Get,
        CMD_PRN_PSaveTime_Set,
        CMD_PRN_PowerOff_Get,
        CMD_PRN_PowerOff_Set,

        CMD_PASSWD_confirmForApply,
        CMD_PASSWD_confirmForSetPasswd,
        CMD_DEVICE_status,

        CMD_WIFI_GetWifiStatus_immediately,

        CMD_WIFI_apply_plus,
        CMD_PASSWD_set_plus,
        CMD_WIFI_refresh_plus,
        CMD_PRN_PowerSave_Get,

        CMD_STATUS_COMPLETE,

        CMD_MAX,
    };
    bool get_confirmed();
    void set_confirmed(bool);

signals:
    void signals_cmd_result(int ,int);

public slots:
    void slots_cmd(int);
    void slots_cmd_plus(int);
    void slots_deviceChanged(const QString& devicename);
};

#endif // DEVICECONTROL_H
