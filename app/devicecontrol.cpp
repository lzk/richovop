/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "devicecontrol.h"
#include"vop_device.h"
#include "vop_protocol.h"
#include "devicemanager.h"
#include "log.h"

QString DeviceContrl::current_devicename = QString();

DeviceContrl::DeviceContrl(DeviceManager* dm):
    device_manager(dm),
    device(dm->device),
    protocol(dm->protocol),
    confirmed(false),
    cmd_status(CMD_STATUS_COMPLETE)
{
}

DeviceContrl::~DeviceContrl()
{
}

int DeviceContrl::device_writeThenRead(char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    if(current_devicename.isEmpty())
        return -1;
    int err = 0;
    QString device_uri = DeviceManager::getDeviceURI(current_devicename);
    err = VopDevice::writeThenRead(device_uri.toLatin1() ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    return err;
}

int DeviceContrl::device_getDeviceStatus(char* buffer ,int buffer_size)
{
    if(current_devicename.isEmpty())
        return -1;
    int err = 0;
    QString device_uri = DeviceManager::getDeviceURI(current_devicename);
    err = VopDevice::getDeviceStatus(device_uri.toLatin1() ,buffer ,buffer_size);
    char str[buffer_size];
    if(!err){
        if(!VopProtocol::getDESfromDeviceID(buffer ,str)){
            qLog(QString("DES:") + str);
            if(VopDevice::getDeviceModel(str) != DeviceManager::getDeviceModel(current_devicename)){
                err = ERR_decode_device;
                qLog("device name and driver name not matched");
            }else{
                qLog("device name and driver name matched");
            }
        }else{
            err = ERR_decode_device;
            qLog("device name and driver name not matched");
        }
    }
    return err;
}

void DeviceContrl::slots_deviceChanged(const QString& devicename)
{
    current_devicename = devicename;
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

    case ERR_communication:
    case ERR_library:
    case ERR_decode_status:
    case ERR_wifi_have_not_been_inited:
    case  ERR_decode_device:
    case  ERR_vop_cannot_support:
        valid = false;
    }
    return valid;
}

int DeviceContrl::cmd_setting_confirm()
{
    int err = 0;
    if(!get_passwd_confirmed()){
        err = protocol->cmd(VopProtocol::CMD_PASSWD_confirm);
        //set confirmed when the cmd complete.
//        if(!err){
//            set_passwd_confirmed(true);
//        }
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
        }
    }
    return err;
}

void DeviceContrl::slots_cmd_plus(int cmd)
{
    if(current_devicename == QString()){
        emit signals_cmd_result(cmd ,-1);
        return;
    }
    int err = -1;
    set_cmdStatus(cmd);
    switch(cmd){
    case CMD_DEVICE_status:
        err = protocol->cmd(VopProtocol::CMD_GetStatus);
        break;

    case CMD_COPY:
        if(!cmd_status_validate(err)){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_COPY);
        break;

    case CMD_WIFI_apply_plus:
        if(!cmd_status_validate(err)){
            break;
        }
        err = cmd_setting_confirm();
        if(err){
            break;
        }
        err = cmd_wifi_status();
        if(err){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_WIFI_apply);
        if(!get_passwd_confirmed())
        if(!err){
            set_passwd_confirmed(true);
        }
        break;

    case CMD_WIFI_refresh_plus:
        if(!cmd_status_validate(err)){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_WIFI_get);
        if(err){
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
        if(!cmd_status_validate(err)){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Get);
        break;
    case CMD_PRN_TonerEnd_Set:
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
        if(!cmd_status_validate(err)){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Get);
        if(err){
            break;
        }
//        err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Get);
        break;
    case CMD_PRN_PSaveTime_Set:
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
        if(!cmd_status_validate(err)){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_NET_GetV4);
        break;
    case CMD_IPv4_Set:
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
        if(!cmd_status_validate(err)){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_NET_GetV6);
        break;
    case CMD_IPv6_Set:
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
    default:
        break;
    }
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
