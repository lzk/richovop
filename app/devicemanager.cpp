/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "devicemanager.h"
#include <cups/cups.h>
#include<QStringList>
#include "vop_device.h"
#include "deviceapp.h"
#include "log.h"
DeviceManager::DeviceManager(MainWidget* _widget):
    device_app(NULL),
    widget(_widget)
{   
    devices.clear();
    device = new VopDevice();
    protocol = new VopProtocol(this);
}

DeviceManager::~DeviceManager()
{
    delete device;
    delete protocol;
    if(device_app)
        delete device_app;
}

const QString DeviceManager::get_deviceName()
{
    return selected_devicename;
}

void DeviceManager::selectDevice(int selected_device)
{
    if(device_app){
        device_app->disconnect_App();
        delete device_app;
        device_app = NULL;
    }
    if(devices.count()){
        selected_devicename = devices.at(selected_device);
        if(!device_app)
            device_app = new DeviceApp(this ,widget);
    }else{
        selected_devicename = QString();
    }
}

QString DeviceManager::getDeviceURI(const QString& devicename)
{
    cups_dest_t *dests;
    int num_dests;
    num_dests = cupsGetDests(&dests);
    cups_dest_t* dest;
    dest = cupsGetDest(devicename.toLatin1() ,NULL ,num_dests ,dests);
    if(!dest)
        return NULL;
    QString device_uri =  cupsGetOption("device-uri", dest->num_options, dest->options);
    cupsFreeDests(num_dests ,dests);
    return device_uri;
}

QString DeviceManager::getCurrentDeviceURI()
{
    return getDeviceURI(selected_devicename);
}

#include<QPrinterInfo>
int DeviceManager::getDeviceList(QStringList& printerInfo)
{
    int selected_printer = 0;
    int default_printer = -1;
    cups_dest_t *dests;
    int num_dests;
    devices.clear();
#if 0
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    devices =QPrinterInfo::availablePrinterNames();
#else
    QList<QPrinterInfo> printerInfoList = QPrinterInfo::availablePrinters();
    for(int i = 0 ; i < printerInfoList.count() ;i++)
    {
        devices.append(printerInfoList[i].printerName());
    }
#endif//(QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
#endif
    num_dests = cupsGetDests(&dests);
    cups_dest_t *dest;
    int i;
    const char *value;
    bool selected = false;
   for (i = num_dests, dest = dests; i > 0; i --, dest ++)
   {
       if (dest->instance == NULL){
           value = cupsGetOption("printer-info", dest->num_options, dest->options);
//            value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options);
//           qLog()<<dest->name<<"---info---"<<value;
           if(VopDevice::isValidDevice(value)){
                devices << dest->name;
                printerInfo << value;
                if(dest->is_default){
                    default_printer = devices.indexOf(dest->name);
                }
                if(( !QString::compare(selected_devicename  ,dest->name))){
                    selected_printer = devices.indexOf(dest->name);
                    selected = true;
                }
                //just display all printer options
//               cups_option_t* option;
//                int j;
//               for(j=0,option=dest->options ;j < dest->num_options ;j++,option++)
//                {
//                    qLog()<<option->name<<option->value;
//                }
           }
//           qLog()<<"";
       }
   }
   if(!devices.count())
   {
       qLog()<<"there is no printer";
       return -1;
   }
   if(selected)
       qLog()<<"the selected printer is founded";
   else if(-1 != default_printer){
           selected_printer =default_printer;
           selected = true;
           qLog()<<"select the default printer";
   }
   if(!selected){
       qLog()<<"the printer isn't selected,select the first printer";
   }
   cupsFreeDests(num_dests, dests);
   return selected_printer;
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

void DeviceManager::copy_set_defaultPara()
{
    protocol->copy_set_defaultPara();
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

cmdst_aplist_get DeviceManager::wifi_getAplist()
{
    return protocol->wifi_getAplist();
}

void DeviceManager::passwd_set(const char* p)
{
    protocol->passwd_set(p);
}

DeviceApp* DeviceManager::deviceApp()
{
    return device_app;
}
