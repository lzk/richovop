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
    static int open();
    static void close();
    static bool isUsbDevice();

private:
    DeviceManager* device_manager;
    VopDevice* device;
    VopProtocol* protocol;
    bool confirmed;
    int cmd_status;
    int cmd_setting_confirm();
    int cmd_wifi_status();
    void set_cmdStatus(int);
    bool cmd_status_validate(int& err);

public:
    enum{
        CMD_DEVICE_status,

        CMD_COPY,
        CMD_WIFI_apply_plus,
        CMD_PASSWD_set_plus,
        CMD_PRN_TonerEnd_Set,
        CMD_PRN_PSaveTime_Set,
        CMD_PRN_PowerOff_Set,

        CMD_WIFI_refresh_plus,
        CMD_PRN_PowerSave_Get,
        CMD_PRN_TonerEnd_Get,

        CMD_IPv4_Get,
        CMD_IPv4_Set,
        CMD_IPv6_Get,
        CMD_IPv6_Set,
        CMD_STATUS_COMPLETE,

        CMD_MAX,
    };
    bool get_passwd_confirmed();
    void set_passwd_confirmed(bool b);
    int get_cmdStatus();

signals:
    void signals_cmd_result(int ,int);
    void signals_progress(int,int);

public slots:
    void slots_cmd_plus(int);
    void slots_deviceChanged(const QString& devicename);
};

#endif // DEVICECONTROL_H
