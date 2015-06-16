/////////////////////////////////////////
/// File:devicemanager.cpp
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "devicemanager.h"
#include<QDebug>
#include<QStringList>
#include"vop_device.h"
#include "vop_protocol.h"

DeviceManager::DeviceManager(QObject *parent) : QObject(parent)
{    
    device = new VopDevice(this);
    devices.clear();
    protocol = new VopProtocol(this);
}

DeviceManager::~DeviceManager()
{
    delete device;
    delete protocol;
}

void DeviceManager::selectDevice(int selected_device)
{
    QMutexLocker locker(&mutex);
    if(devices.count()){
        selected_devicename = devices.at(selected_device);
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
    mutex.lock();
    current_devicename = selected_devicename;
    mutex.unlock();
    return getDeviceURI(current_devicename);
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
//           qDebug()<<dest->name<<"---info---"<<value;
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
//                    qDebug()<<option->name<<option->value;
//                }
           }
//           qDebug()<<"";
       }
   }
   if(!devices.count())
   {
       qDebug()<<"there is no printer";
       return -1;
   }
   if(selected)
       qDebug()<<"the selected printer is founded";
   else if(-1 != default_printer){
           selected_printer =default_printer;
           selected = true;
           qDebug()<<"select the default printer";
   }
   if(!selected){
       qDebug()<<"the printer isn't selected,select the first printer";
   }
   //select the device
   selectDevice(selected_printer);   
   cupsFreeDests(num_dests, dests);
   return selected_printer;
}

QString DeviceManager::current_devicename = QString();
int DeviceManager::device_writeThenRead(char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    if(current_devicename.isEmpty())
        return -1;
    int err = 0;
    QString device_uri = getDeviceURI(current_devicename);
    err = VopDevice::writeThenRead(device_uri.toLatin1() ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    return err;
}

void DeviceManager::slots_cmd(int cmd)
{
//    assert(cmd < CMD_MAX);
    mutex.lock();
    current_devicename = selected_devicename;
    mutex.unlock();
    if(current_devicename == QString())
        return;
    int err = -1;
    switch(cmd){
    case CMD_DEVICE_status:    {
        QString device_uri = getDeviceURI(current_devicename);
        emit signals_setProgress(0);
        emit signals_setProgress(20);
        err = VopDevice::getDeviceStatus(device_uri.toLatin1());
//        sleep(5);
        emit signals_setProgress(100);
        break;
    }
    case CMD_COPY:
        err = protocol->cmd(VopProtocol::CMD_COPY);
        break;
    case CMD_WIFI_apply:
        err = protocol->cmd(VopProtocol::CMD_WIFI_apply);
        break;
    case CMD_WIFI_get:
        err = protocol->cmd(VopProtocol::CMD_WIFI_get);
        break;
    case CMD_WIFI_getAplist:
        err = protocol->cmd(VopProtocol::CMD_WIFI_getAplist);
        break;
    case CMD_PASSWD_set:
        err = protocol->cmd(VopProtocol::CMD_PASSWD_set);
        break;
    case CMD_PASSWD_get:
        err = protocol->cmd(VopProtocol::CMD_PASSWD_get);
        break;
    case CMD_PASSWD_confirm:
    case CMD_PASSWD_confirmForApply:
    case CMD_PASSWD_confirmForSetPasswd:
        err = protocol->cmd(VopProtocol::CMD_PASSWD_confirm);
        break;
    default:
        break;
    }
    emit signals_cmd_result(cmd ,err);
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
