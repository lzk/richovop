/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QStringList>
#include <QMutex>
#include "vop_protocol.h"
class Device;
class MainWidget;
class DeviceApp;
class DeviceManager
{
public:
    DeviceManager(MainWidget* widget);
    ~DeviceManager();
    QMutex mutex_ctrl;
    QMutex mutex_app;
    static QMutex mutex;

    DeviceApp* deviceApp();

    int getDeviceList(QStringList& printerNames);
    void selectDevice(int selected_device);
    QString getCurrentDeviceURI();
    static QString getDeviceURI(const QString&);
    const QString get_deviceName();
    static int getDeviceModel(const QString& devicename);
    static QString getStringFromShell(const QString& cmd ,int mode = 0);

    int get_deviceStatus();

    void copy_set_defaultPara(copycmdset* p);
    void copy_set_para(copycmdset* p);
    copycmdset copy_get_para();

    void wifi_set_ssid(cmdst_wifi_get*  ,const char*);
    void wifi_set_password(cmdst_wifi_get*  ,const char*);
    void wifi_set_para(cmdst_wifi_get* p);
    cmdst_wifi_get wifi_get_para();
    cmdst_aplist_get wifi_getAplist();
    cmdst_wifi_status wifi_getWifiStatus();
    cmdst_tonerEnd printer_getTonerEnd();
    void printer_setTonerEnd(cmdst_tonerEnd*);
    cmdst_PSave_time printer_getPSaveTime();
    void printer_setPSaveTime(cmdst_PSave_time*);
    cmdst_powerOff_time printer_getPowerOffTime();
    void printer_setPowerOffTime(cmdst_powerOff_time*);
    net_info_st net_getIpv4info();
    void net_setIpv4info(net_info_st* p);
    net_ipv6_st net_getIpv6info();
    void net_setIpv6info(net_ipv6_st* p);

    void passwd_set(const char*);

//    void emit_cmd(int);
    void emit_cmd_plus(int);

    void set_tmp_passwd(const char*);
    void load_tmp_passwd_to_set();
    bool get_passwd_confirmed();
    void set_passwd_confirmed(bool b);

private:
    QStringList devices;
    QString selected_devicename;
    QString passwd_to_set;
    DeviceApp* device_app;
    MainWidget* main_widget;

public:
    Device* usb_device;
    Device* net_device;
    VopProtocol* protocol;
};

#endif // DEVICEMANAGER_H
