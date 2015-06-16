/////////////////////////////////////////
/// File:devicemanager.h
/// Author:Jacky Liang
/// Version:
////////////////////////////////////////
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>

#include <cups/cups.h>
#include <QMutex>
#include <QStringList>

class VopDevice;
#include "vop_protocol.h"
class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = 0);
    ~DeviceManager();

    static QString current_devicename;
    QMutex mutex;

    int getDeviceList(QStringList& printerInfo);
    void selectDevice(int selected_device);
    QString getCurrentDeviceURI();
    static QString getDeviceURI(const QString&);
    static int device_writeThenRead(char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize);

    void copy_set_defaultPara();
    void copy_set_para(copycmdset* p);
    copycmdset copy_get_para();

//    char* wifi_get_ssid();
    void wifi_set_ssid(cmdst_wifi_get*  ,const char*);
//    char* wifi_get_password();
    void wifi_set_password(cmdst_wifi_get*  ,const char*);
    void wifi_set_para(cmdst_wifi_get* p);
    cmdst_wifi_get wifi_get_para();
    cmdst_aplist_get wifi_getAplist();

    void passwd_set(const char*);

private:
    VopDevice* device;
    QStringList devices;
    QString selected_devicename;
    VopProtocol* protocol;


public:
    enum{
        CMD_COPY,
        CMD_WIFI_apply,
        CMD_WIFI_get,
        CMD_WIFI_getAplist,
        CMD_PASSWD_set,
        CMD_PASSWD_get,
        CMD_PASSWD_confirm,

        CMD_PASSWD_confirmForApply,
        CMD_PASSWD_confirmForSetPasswd,
        CMD_DEVICE_status,

        CMD_STATUS_COMPLETE,

        CMD_MAX,
    };

signals:
    void signals_cmd_result(int ,int);
    void signals_setProgress(int);

public slots:
    void slots_cmd(int);
};

#endif // DEVICEMANAGER_H
