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

int DeviceContrl::cmd_setting_confirm()
{
    int err = 0;
    if(!get_passwd_confirmed()){
        err = protocol->cmd(VopProtocol::CMD_PASSWD_confirm);
        if(!err){
            set_passwd_confirmed(true);
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
        err = protocol->cmd(VopProtocol::CMD_COPY);
        break;

    case CMD_WIFI_apply_plus:
        err = cmd_setting_confirm();
        if(err){
            break;
        }
        err = cmd_wifi_status();
        if(err){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_WIFI_apply);
        break;

    case CMD_WIFI_refresh_plus:
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
        err = protocol->cmd(VopProtocol::CMD_PASSWD_confirm);
        if(err){
            break;
        }
        emit signals_progress(cmd ,50);
        set_passwd_confirmed(false);
        //load passwd
        device_manager->load_tmp_passwd_to_set();
        err = protocol->cmd(VopProtocol::CMD_PASSWD_set);
        break;

    case CMD_PRN_TonerEnd_Get:
        err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Get);
        break;
    case CMD_PRN_TonerEnd_Set:
        err = cmd_setting_confirm();
        if(err){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Set);
        break;
    case CMD_PRN_PowerSave_Get:
        err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Get);
        if(err){
            break;
        }
//        err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Get);
        break;
    case CMD_PRN_PSaveTime_Set:
        err = cmd_setting_confirm();
        if(err){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Set);
        break;
    case CMD_PRN_PowerOff_Set:
        err = cmd_setting_confirm();
        if(err){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Set);
        break;
    case CMD_IPv4_Get:
        err = protocol->cmd(VopProtocol::CMD_NET_GetV4);
        break;
    case CMD_IPv4_Set:
        err = cmd_setting_confirm();
        if(err){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_NET_SetV4);
        break;
    case CMD_IPv6_Get:
        err = protocol->cmd(VopProtocol::CMD_NET_GetV6);
        break;
    case CMD_IPv6_Set:
        err = cmd_setting_confirm();
        if(err){
            break;
        }
        err = protocol->cmd(VopProtocol::CMD_NET_SetV6);
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
