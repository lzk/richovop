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
    VopDevice* device;
    VopProtocol* protocol;
//    DeviceManager* device_manager;


public:
    enum{
        CMD_COPY,
        CMD_WIFI_apply,
        CMD_WIFI_get,
        CMD_WIFI_getAplist,
        CMD_PASSWD_set,
        CMD_PASSWD_get,
        CMD_PASSWD_confirm,
        CMD_WIFI_GetWifiStatus,

        CMD_PASSWD_confirmForApply,
        CMD_PASSWD_confirmForSetPasswd,
        CMD_DEVICE_status,

        CMD_STATUS_COMPLETE,

        CMD_MAX,
    };

signals:
    void signals_cmd_result(int ,int);

public slots:
    void slots_cmd(int);
    void slots_deviceChanged(const QString& devicename);
};

#endif // DEVICECONTROL_H
