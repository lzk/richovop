/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QStringList>
#include <QMutex>
#include "vop_protocol.h"
class VopDevice;
class MainWidget;
class DeviceApp;
class DeviceManager
{
public:
    DeviceManager(MainWidget* widget);
    ~DeviceManager();
    VopDevice* device;
    VopProtocol* protocol;
    QMutex mutex_ctrl;
    QMutex mutex_app;

    DeviceApp* deviceApp();

    int getDeviceList(QStringList& printerNames);
    void selectDevice(int selected_device);
    QString getCurrentDeviceURI();
    static QString getDeviceURI(const QString&);
    const QString get_deviceName();

    int get_deviceStatus();

    void copy_set_defaultPara();
    void copy_set_para(copycmdset* p);
    copycmdset copy_get_para();

    void wifi_set_ssid(cmdst_wifi_get*  ,const char*);
    void wifi_set_password(cmdst_wifi_get*  ,const char*);
    void wifi_set_para(cmdst_wifi_get* p);
    cmdst_wifi_get wifi_get_para();
    cmdst_aplist_get wifi_getAplist();

    void passwd_set(const char*);
private:
    QStringList devices;
    QString selected_devicename;
    DeviceApp* device_app;
    MainWidget* widget;
};

#endif // DEVICEMANAGER_H
