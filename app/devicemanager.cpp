/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "devicemanager.h"
#include<QStringList>
#include "device.h"
#include "deviceapp.h"
#include "log.h"
#include "usbdevice.h"
#include "netdevice.h"
#include "devicedata.h"
#include "linux_api.h"
DeviceManager::DeviceManager(MainWidget* widget):
    device_app(NULL),
    main_widget(widget),
    usb_device(new UsbDevice),
    net_device(new NetDevice),
    device_data(new DeviceData)
{
    devices.clear();
    protocol = new VopProtocol(this);
}

DeviceManager::~DeviceManager()
{
    if(device_app)
        delete device_app;
    delete protocol;
    delete usb_device;
    delete net_device;
    delete device_data;
}

const QString DeviceManager::get_deviceName()
{
    return selected_devicename;
}

bool DeviceManager::selectDevice(int selected_device)
{
    bool changed = false;
    if(!devices.isEmpty() && -1 != selected_device){
        if(selected_devicename.compare(devices.at(selected_device))){
            _Q_LOG("select another device");
            changed = true;
            selected_devicename = devices.at(selected_device);
            if(device_app){
                device_app->disconnect_App();
                delete device_app;
                device_app = NULL;
            }
            device_data->clear();
            device_app = new DeviceApp(this ,main_widget);
        }
    }else{
        _Q_LOG("no device");
        if(!selected_devicename.isEmpty()){
            changed = true;
            if(device_app){
                device_app->disconnect_App();
                delete device_app;
                device_app = NULL;
            }
            device_data->clear();
            selected_devicename = QString();
        }
    }
    return changed;
}

QString DeviceManager::getCurrentDeviceURI()
{
    return get_device_uri(selected_devicename);
}

int DeviceManager::getDeviceList(QStringList& printerNames)
{
    int selected_printer = 0;
    int default_printer = -1;
    devices.clear();

    QStringList printers = get_printers();
    if(printers.isEmpty()){
        _Q_LOG("there is no printer");
        return -1;
    }
//    _Q_LOG("printers are:" + printers.join(";"));
    QString print;
    foreach (print, printers) {
        if(!print.isEmpty()){
                if(Device::Device_invalid != getDeviceModel(print)){
                    devices << print;
                    printerNames << print;
                }
        }
    }
   if(devices.isEmpty())
   {
       _Q_LOG("there is no printer");
       return -1;
   }

   //get default printer
   QString default_printer_name = get_default_printer();

   default_printer = devices.indexOf(default_printer_name);   
   selected_printer = devices.indexOf(selected_devicename);
   if(-1 != selected_printer)
       _Q_LOG("the selected printer is founded");
   else if(-1 != default_printer){
           selected_printer =default_printer;
           _Q_LOG("select the default printer");
   }
   if(-1 == selected_printer){
       _Q_LOG("the printer isn't selected,select the first printer");
       selected_printer = 0;
   }
   return selected_printer;
}

int DeviceManager::getDeviceModel(const QString& devicename)
{
    QString makeAndModel;
    makeAndModel = get_device_model(devicename);
    if(makeAndModel.isEmpty())
        return Device::Device_invalid;
    _Q_LOG(devicename + " make and model is " + makeAndModel);
    return Device::getDeviceModel(makeAndModel.toLatin1());
}

int DeviceManager::get_deviceStatus()
{
    return protocol->get_deviceStatus();
}

void DeviceManager::copy_set_para(copycmdset* p)
{
    protocol->copy_set_para(p);
}

copycmdset DeviceManager::copy_get_para()
{
    return protocol->copy_get_para();
}

void DeviceManager::copy_set_defaultPara(copycmdset* p)
{
    protocol->copy_set_defaultPara(p);
}

void DeviceManager::wifi_set_ssid(cmdst_wifi_get* p ,const char* ssid)
{
    protocol->wifi_set_ssid(p ,ssid);
}

void DeviceManager::wifi_set_password(cmdst_wifi_get* p ,const char* password)
{
    protocol->wifi_set_password(p ,password);
}

void DeviceManager::wifi_set_para(cmdst_wifi_get* p)
{
    protocol->wifi_set_para(p);
}

cmdst_wifi_get DeviceManager::wifi_get_para()
{
    return protocol->wifi_get_para();
}

cmdst_wifi_status DeviceManager::wifi_getWifiStatus()
{
    return protocol->wifi_getWifiStatus();
}

cmdst_aplist_get DeviceManager::wifi_getAplist()
{
    return protocol->wifi_getAplist();
}

void DeviceManager::passwd_set(const char* p)
{
    protocol->passwd_set(p);
}

cmdst_tonerEnd DeviceManager::printer_getTonerEnd()
{
    return protocol->printer_getTonerEnd();
}

void DeviceManager::printer_setTonerEnd(cmdst_tonerEnd* p)
{
    protocol->printer_setTonerEnd(p);
}

cmdst_PSave_time DeviceManager::printer_getPSaveTime()
{
    return  protocol->printer_getPSaveTime();
}

void DeviceManager::printer_setPSaveTime(cmdst_PSave_time* p)
{
    protocol->printer_setPSaveTime(p);
}

cmdst_powerOff_time DeviceManager::printer_getPowerOffTime()
{
    return  protocol->printer_getPowerOffTime();
}

void DeviceManager::printer_setPowerOffTime(cmdst_powerOff_time* p)
{
    protocol->printer_setPowerOffTime(p);
}

net_info_st DeviceManager::net_getIpv4info()
{
    return protocol->net_getV4();
}

void DeviceManager::net_setIpv4info(net_info_st* p)
{
    protocol->net_setV4(p);
}

net_ipv6_st DeviceManager::net_getIpv6info()
{
    return protocol->net_getV6();
}

void DeviceManager::net_setIpv6info(net_ipv6_st* p)
{
    protocol->net_setV6(p);
}

DeviceApp* DeviceManager::deviceApp()
{
    return device_app;
}

void DeviceManager::emit_cmd_plus(int cmd)
{
    if(device_app)
        device_app->emit_cmd_plus(cmd);
}

void DeviceManager::set_tmp_passwd(const char* p)
{
    QMutexLocker locker(&mutex_ctrl);
    passwd_to_set = p;
}

void DeviceManager::load_tmp_passwd_to_set()
{
    QString str;
    mutex_ctrl.lock();
    str = passwd_to_set;
    mutex_ctrl.unlock();
    passwd_set(str.toLatin1());
}

bool DeviceManager::get_passwd_confirmed()
{
    if(device_app)
        return device_app->get_passwd_confirmed();
    else
        return false;
}

void DeviceManager::set_passwd_confirmed(bool b)
{
    if(device_app)
        device_app->set_passwd_confirmed(b);
}
