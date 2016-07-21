/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "devicecontrol.h"
#include"device.h"
#include "vop_protocol.h"
#include "devicemanager.h"
#include "log.h"
#include "usbdevice.h"
#include "netdevice.h"
#include "linux_api.h"
#include <QTime>

Device* DeviceContrl::device = NULL;
QString DeviceContrl::current_devicename = QString();

DeviceContrl::DeviceContrl(DeviceManager* dm):
    device_manager(dm),
    usb_device(dm->usb_device),
    net_device(dm->net_device),
    protocol(dm->protocol),
    confirmed(false),
    cmd_status(CMD_STATUS_COMPLETE)
{
}

DeviceContrl::~DeviceContrl()
{
}

bool DeviceContrl::isUsbDevice()
{
    return (device == usb_device);
}
bool DeviceContrl::isNetDevice()
{
    return (device == net_device);
}

int DeviceContrl::openPrinter()
{
    if(current_devicename.isEmpty() || !device)
        return ERR_communication;
    QString device_uri = get_device_uri(current_devicename);
    return device->openPrinter(device_uri.toLatin1());
}

void DeviceContrl::closePrinter()
{
    if(!current_devicename.isEmpty() && device){
        device->closePrinter();
    }
}

int DeviceContrl::device_write_no_Read(char* wrBuffer ,int wrSize)
{
    if(current_devicename.isEmpty() || !device)
        return ERR_communication;
    return device->write_no_read(wrBuffer ,wrSize);
}

int DeviceContrl::device_writeThenRead(char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    if(current_devicename.isEmpty() || !device)
        return ERR_communication;
    return device->write_then_read(wrBuffer ,wrSize ,rdBuffer ,rdSize);
}

int DeviceContrl::device_getDeviceStatus(char* buffer ,int buffer_size)
{
    if(current_devicename.isEmpty() || !device)
        return ERR_communication;
    int err = device->get_device_id(buffer ,buffer_size);
    char str[buffer_size];
    if(!err){
        if(!VopProtocol::getDESfromDeviceID(buffer ,str)){
            if(Device::getDeviceModel(str) != DeviceManager::getDeviceModel(current_devicename)){
                _Q_LOG("device name and driver name not matched");
                _Q_LOG(QString("DES:") + str);
                err = ERR_decode_device;
            }
        }else{
            err = ERR_decode_device;
            _Q_LOG("device name and driver name not matched");
            _Q_LOG("can not get DES");
        }
    }
    return err;
}

#include <QUrl>
void DeviceContrl::slots_deviceChanged(const QString& devicename)
{
    _Q_LOG("");
    _Q_LOG("");
    _Q_LOG("current device name:" + devicename);
    current_devicename = devicename;
    if(current_devicename.isEmpty()){
        device = NULL;
    }else{
        QString device_uri = get_device_uri(current_devicename);
        _Q_LOG("current device uri:" + device_uri);

        if(device_uri.startsWith("usb://")
                || device_uri.startsWith("hal://")
                ){
            device = usb_device;
            _Q_LOG("it is usb device");
        }else if(device_uri.startsWith("socket://")
                         || device_uri.startsWith("dnssd://")
                         || device_uri.startsWith("lpd://")
                         || device_uri.startsWith("ipp://")
                         || device_uri.startsWith("lpr://")
                         || device_uri.startsWith("http://")
                         || device_uri.startsWith("https://")
             //            || device_uri.startsWith("mdns://")
                 ){
            device = net_device;
            _Q_LOG("it is net device");
        }else{
            device = NULL;
            _Q_LOG("can not support the uri");
        }

 /*   //sometimes can not get scheme
        QString scheme = QUrl(device_uri).scheme();
        if(!scheme.compare("usb")
                || !scheme.compare("hal")
                ){
            device = usb_device;
            _Q_LOG("it is usb device");
        }else if(!scheme.compare("socket")
                         || !scheme.compare("dnssd")
                         || !scheme.compare("lpd")
                         || !scheme.compare("ipp")
                         || !scheme.compare("lpr")
             //            || !scheme.compare("mdns")
                 ){
            device = net_device;
            _Q_LOG("it is net device");
        }else{
            device = NULL;
            _Q_LOG("can not support the scheme:" + scheme);
        }
        //*/
    }
}

bool DeviceContrl::get_passwd_confirmed()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return confirmed;
}
void DeviceContrl::set_passwd_confirmed(bool b)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    confirmed = b;
}

bool DeviceContrl::cmd_status_validate(int& err)
{
    bool valid = true;
    err = protocol->cmd(VopProtocol::CMD_GetStatus);
    if(err > 0){
        valid = false;
    }
    switch(err){
    case STATUS_busy_printing:
    case STATUS_busy_scanningOrCoping:
    case STATUS_jam:
    case STATUS_CopyScanNextPage:
    case STATUS_IDCardCopyTurnCardOver:

    case ERR_communication:
    case ERR_library:
    case ERR_decode_status:
    case ERR_wifi_have_not_been_inited:
    case  ERR_decode_device:
    case  ERR_vop_cannot_support:
        valid = false;
        _Q_LOG("err: status is invalid");
        break;
    default:
        break;
    }
    return valid;
}

bool DeviceContrl::cmd_status_validate_copy(int& err)
{
    bool valid = true;
    err = protocol->cmd(VopProtocol::CMD_GetStatus);
    if(err > 0){
        valid = false;
    }
    switch(err){
    case STATUS_TonerEnd:
    case STATUS_busy_printing:
    case STATUS_busy_scanningOrCoping:
    case STATUS_jam:
    case STATUS_CopyScanNextPage:
    case STATUS_IDCardCopyTurnCardOver:

    case ERR_communication:
    case ERR_library:
    case ERR_decode_status:
    case ERR_wifi_have_not_been_inited:
    case  ERR_decode_device:
    case  ERR_vop_cannot_support:
        valid = false;
        _Q_LOG("err: status is invalid");
        break;
    default:
        break;
    }
    return valid;
}
int DeviceContrl::cmd_setting_confirm()
{
    int err = 0;
    if(!get_passwd_confirmed()){
        err = protocol->cmd(VopProtocol::CMD_PASSWD_confirm);
        //set confirmed when the cmd complete.
        if(!err){
//            set_passwd_confirmed(true);
            _Q_LOG("err: password not confirmed");
        }
    }
    return err;
}

int DeviceContrl::cmd_wifi_status()
{
    int err;
    err = protocol->cmd(VopProtocol::CMD_WIFI_GetWifiStatus);
    if(!err){
        cmdst_wifi_status wifi_status = device_manager->wifi_getWifiStatus();
        if(1 != wifi_status){
            //wifi don't init
            err = ERR_wifi_have_not_been_inited;
            _Q_LOG("err: wifi have not been inited");
        }
    }
    return err;
}

#include <unistd.h>
void DeviceContrl::slots_cmd_plus(int cmd)
{
    if(current_devicename == QString()){
        emit signals_cmd_result(cmd ,-1);
        return;
    }

    int err = ERR_ACK;
    set_cmdStatus(cmd);

    _Q_LOG("");
    _Q_LOG("");

    QTime time;
    time.start();

    err = DeviceContrl::openPrinter();
    if(err){
        set_cmdStatus(CMD_STATUS_COMPLETE);
        emit signals_cmd_result(cmd ,err);
        return;
    }

//    if(isUsbDevice() && scanner_locked()){
    if(isUsbDevice()){
        if(UsbDevice::is_device_scanning()
            || UsbDevice::is_Airprint_scanning()){
            err = ERR_sane_scanning;
            _Q_LOG("err: usb device scanner locked");
        }
    }

    if(isUsbDevice() && !err){
        if(!get_printer_jobs(current_devicename)
                ){
            err = ERR_ACK;
        }else{
            _Q_LOG("err: printer have jobs");
            err = ERR_printer_have_jobs;
        }
    }

    if(!err){
        switch(cmd){
        case CMD_DEVICE_status:
            _Q_LOG("exec control cmd: get device status");
            err = protocol->cmd(VopProtocol::CMD_GetStatus);
            break;
        case CMD_Device_GetFirstStatus:
            _Q_LOG("exec control cmd: get first device status");
            err = protocol->cmd(VopProtocol::CMD_GetStatus);
            break;

        case CMD_COPY:
            _Q_LOG("exec control cmd: copy");
            if(!cmd_status_validate_copy(err)){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_COPY);
            break;

        case CMD_WIFI_apply_plus:
            _Q_LOG("exec control cmd: wifi apply");
            if(!cmd_status_validate(err)){
                break;
            }
            err = cmd_setting_confirm();
            if(err){
                break;
            }
//            err = cmd_wifi_status();
//            if(err){
//                break;
//            }
            if(isNetDevice())
                err = protocol->cmd(VopProtocol::CMD_WIFI_apply_noread);
            else
                err = protocol->cmd(VopProtocol::CMD_WIFI_apply);
            if(!get_passwd_confirmed())
            if(!err){
                set_passwd_confirmed(true);
            }
            break;

        case CMD_WIFI_refresh_plus:
            _Q_LOG("exec control cmd: wifi refresh");
            if(!cmd_status_validate(err)){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_WIFI_get);
            if(err){
                _Q_LOG("err: can not get wifi");
                break;
            }
            err = cmd_wifi_status();
            if(err){
                break;
            }
            emit signals_progress(cmd ,50);
            err = protocol->cmd(VopProtocol::CMD_WIFI_getAplist);
            break;

        case CMD_PASSWD_set_plus:
            _Q_LOG("exec control cmd: set passwd");
            if(!cmd_status_validate(err)){
                break;
            }
            err = cmd_setting_confirm();
            if(err){
                break;
            }
            emit signals_progress(cmd ,50);
    //        set_passwd_confirmed(false);
            //load passwd
            device_manager->load_tmp_passwd_to_set();
            err = protocol->cmd(VopProtocol::CMD_PASSWD_set);
            if(!get_passwd_confirmed())
            if(!err){
                set_passwd_confirmed(true);
            }
            break;

        case CMD_PRN_TonerEnd_Get:
            _Q_LOG("exec control cmd: get toner end");
            if(!cmd_status_validate(err)){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Get);
            break;
        case CMD_PRN_TonerEnd_Set:
            _Q_LOG("exec control cmd: set toner end");
            if(!cmd_status_validate(err)){
                break;
            }
            err = cmd_setting_confirm();
            if(err){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Set);
            if(!get_passwd_confirmed())
            if(!err){
                set_passwd_confirmed(true);
            }
            break;
        case CMD_PRN_PowerSave_Get:
            _Q_LOG("exec control cmd: get power save time");
            if(!cmd_status_validate(err)){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Get);
            if(err){
                _Q_LOG("err: can not set save time");
                break;
            }
    //        err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Get);
            break;
        case CMD_PRN_PSaveTime_Set:
            _Q_LOG("exec control cmd: set power save time");
            if(!cmd_status_validate(err)){
                break;
            }
            err = cmd_setting_confirm();
            if(err){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Set);
            if(!get_passwd_confirmed())
            if(!err){
                set_passwd_confirmed(true);
            }
            break;
        case CMD_PRN_PowerOff_Set:
            _Q_LOG("exec control cmd: set pwoer off time");
            if(!cmd_status_validate(err)){
                break;
            }
            err = cmd_setting_confirm();
            if(err){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Set);
            if(!get_passwd_confirmed())
            if(!err){
                set_passwd_confirmed(true);
            }
            break;
        case CMD_IPv4_Get:
            _Q_LOG("exec control cmd: get ipv4");
            if(!cmd_status_validate(err)){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_NET_GetV4);
            break;
        case CMD_IPv4_Set:
            _Q_LOG("exec control cmd: set ipv4");
            if(!cmd_status_validate(err)){
                break;
            }
            err = cmd_setting_confirm();
            if(err){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_NET_SetV4);
            if(!get_passwd_confirmed())
            if(!err){
                set_passwd_confirmed(true);
            }
            break;
        case CMD_IPv6_Get:
            _Q_LOG("exec control cmd: get ipv6");
            if(!cmd_status_validate(err)){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_NET_GetV6);
            break;
        case CMD_IPv6_Set:
            _Q_LOG("exec control cmd: set ipv6");
            if(!cmd_status_validate(err)){
                break;
            }
            err = cmd_setting_confirm();
            if(err){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_NET_SetV6);
            if(!get_passwd_confirmed())
            if(!err){
                set_passwd_confirmed(true);
            }
            break;
        case CMD_PRN_GetRegion:{
            _Q_LOG("exec control cmd: get region");
            if(!cmd_status_validate(err)){
                break;
            }
            err = protocol->cmd(VopProtocol::CMD_PRN_GetRegion);
            break;
        }
        default:
            break;
        }
    }
    DeviceContrl::closePrinter();
    C_LOG("elapsed time:%d" , time.elapsed());
    emit signals_progress(cmd ,80);
    set_cmdStatus(CMD_STATUS_COMPLETE);
    emit signals_cmd_result(cmd ,err);
}

void DeviceContrl::set_cmdStatus(int status)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    cmd_status = status;
}

int DeviceContrl::get_cmdStatus()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return cmd_status;
}
