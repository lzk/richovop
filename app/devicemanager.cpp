/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "devicemanager.h"
#include<QStringList>
#include "vop_device.h"
#include "deviceapp.h"
#include "log.h"
#include <cups/cups.h>
DeviceManager::DeviceManager(MainWidget* widget):
    device_app(NULL),
    main_widget(widget)
{
    devices.clear();
    device = new VopDevice();
    protocol = new VopProtocol(this);
}

DeviceManager::~DeviceManager()
{
    if(device_app)
        delete device_app;
    delete device;
    delete protocol;
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
    if(!devices.isEmpty() && -1 != selected_device){
        selected_devicename = devices.at(selected_device);
        if(!device_app)
            device_app = new DeviceApp(this ,main_widget);
    }else{
        selected_devicename = QString();
    }
}
#include<QFile>
#include<QTextStream>
QString DeviceManager::getDeviceURI(const QString& devicename)
{
    QString device_uri;

    //rhel5 not work
    cups_dest_t *dests;
    int num_dests;
    num_dests = cupsGetDests(&dests);
    cups_dest_t* dest;
    dest = cupsGetDest(devicename.toLatin1() ,NULL ,num_dests ,dests);
    if(!dest)
        return NULL;
    device_uri =  cupsGetOption("device-uri", dest->num_options, dest->options);
    cupsFreeDests(num_dests ,dests);

    if(1){
//    if(device_uri.isEmpty()){
        QString str("LANG=en_US.UTF-8 lpstat -v ");
        QString tmp_file("/tmp/lpstattmpfile123456789");
        str += devicename;
//        str += "|awk \'{print $NF}\'";
        str += ">";
        str += tmp_file;
        if(!system(str.toLatin1())){
            QFile fl(tmp_file);
            if(fl.open(QFile::ReadOnly)){
                QTextStream in(&fl);
                device_uri = in.readLine();
                device_uri = device_uri.section(' ' ,-1);
                fl.close();
                fl.remove();
            }
        }
    }
    return device_uri;
}

QString DeviceManager::getCurrentDeviceURI()
{
    return getDeviceURI(selected_devicename);
}

//#include<QPrinterInfo>
///
/// \brief DeviceManager::getDeviceList :get printer names and set selected printer
/// \param printerNames :return printer names
/// \return :selected for current printer
///
int DeviceManager::getDeviceList(QStringList& printerNames)
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
            value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options);
           if(VopDevice::isValidDevice(value)){
                devices << dest->name;
                printerNames << dest->name;
                if(dest->is_default){
                    default_printer = devices.indexOf(dest->name);
                }
                if(( !QString::compare(selected_devicename  ,dest->name))){
                    selected_printer = devices.indexOf(dest->name);
                    selected = true;
                }
           }
       }
   }
   if(devices.isEmpty())
   {
       qLog("there is no printer");
       return -1;
   }
   if(selected)
       qLog("the selected printer is founded");
   else if(-1 != default_printer){
           selected_printer =default_printer;
           selected = true;
           qLog("select the default printer");
   }
   if(!selected){
       qLog("the printer isn't selected,select the first printer");
   }
   cupsFreeDests(num_dests, dests);
   return selected_printer;
}

int DeviceManager::getDeviceModel(const QString& devicename)
{
    const char* value;
    cups_dest_t *dests;
    int num_dests;
    num_dests = cupsGetDests(&dests);
    cups_dest_t* dest;
    dest = cupsGetDest(devicename.toLatin1() ,NULL ,num_dests ,dests);
    if(!dest)
        return VopDevice::Device_invalid;
    value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options);
    int model =  VopDevice::isValidDevice(value);
    cupsFreeDests(num_dests ,dests);
    return model;
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
}/*

void DeviceManager::emit_cmd(int cmd)
{
    if(device_app){
        if(device_app->emit_cmd(cmd)){
//            messagebox_exec(tr("The machine is busy, please try later..."));
        }
    }
}*/

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
