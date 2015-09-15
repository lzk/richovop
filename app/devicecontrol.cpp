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
    confirmed(false)
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

void DeviceContrl::slots_cmd(int cmd)
{
//    assert(cmd < CMD_MAX);
//    mutex.lock();
//    current_devicename = selected_devicename;
//    mutex.unlock();
    if(current_devicename == QString())
        return;
    int err = -1;
    switch(cmd){
    case CMD_DEVICE_status:
        err = protocol->cmd(VopProtocol::CMD_GetStatus);
        break;
    case CMD_GetCopy:
        err = protocol->cmd(VopProtocol::CMD_GetCopy);
        break;
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
    case CMD_WIFI_GetWifiStatus:
    case CMD_WIFI_GetWifiStatus_immediately:
        err = protocol->cmd(VopProtocol::CMD_WIFI_GetWifiStatus);
        break;
    case CMD_PASSWD_confirm:
    case CMD_PASSWD_confirmForApply:
    case CMD_PASSWD_confirmForSetPasswd:
        err = protocol->cmd(VopProtocol::CMD_PASSWD_confirm);
        break;
    case CMD_PRN_TonerEnd_Get:
        err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Get);
        break;
    case CMD_PRN_TonerEnd_Set:
        err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Set);
        break;
    case CMD_PRN_PSaveTime_Get:
        err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Get);
        break;
    case CMD_PRN_PSaveTime_Set:
        err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Set);
        break;
    case CMD_PRN_PowerOff_Get:
        err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Get);
        break;
    case CMD_PRN_PowerOff_Set:
        err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Set);
        break;
    default:
        break;
    }
    emit signals_cmd_result(cmd ,err);
}

bool DeviceContrl::get_confirmed()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return confirmed;
}
void DeviceContrl::set_confirmed(bool b)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    confirmed = b;
}

int DeviceContrl::cmd_setting_confirm()
{
    int err;
    if(!get_confirmed())
        err = protocol->cmd(VopProtocol::CMD_PASSWD_confirm);
    if(!err){
        set_confirmed(true);
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
            emit signals_cmd_result(cmd ,err);
            return;
        }
        err = cmd_wifi_status();
        if(err){
            emit signals_cmd_result(cmd ,err);
            return;
        }
        err = protocol->cmd(VopProtocol::CMD_WIFI_apply);
        emit signals_cmd_result(cmd ,err);
        break;

    case CMD_WIFI_refresh_plus:
        err = protocol->cmd(VopProtocol::CMD_WIFI_get);
        if(err){
            emit signals_cmd_result(cmd ,err);
            return;
        }
        err = cmd_wifi_status();
        if(err){
            emit signals_cmd_result(cmd ,err);
            return;
        }
        err = protocol->cmd(VopProtocol::CMD_WIFI_getAplist);
        emit signals_cmd_result(cmd ,err);
        break;

    case CMD_PASSWD_set_plus:
        err = protocol->cmd(VopProtocol::CMD_PASSWD_confirm);
        if(err){
            emit signals_cmd_result(cmd ,err);
            return;
        }
        set_confirmed(false);
        //load passwd
        device_manager->load_tmp_passwd_to_set();
        err = protocol->cmd(VopProtocol::CMD_PASSWD_set);
        emit signals_cmd_result(cmd ,err);
        break;

    case CMD_PRN_TonerEnd_Get:
        err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Get);
        emit signals_cmd_result(cmd ,err);
        break;
    case CMD_PRN_TonerEnd_Set:
        err = protocol->cmd(VopProtocol::CMD_PRN_TonerEnd_Set);
        emit signals_cmd_result(cmd ,err);
        break;
    case CMD_PRN_PowerSave_Get:
        err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Get);
        if(err){
            emit signals_cmd_result(cmd ,err);
            return;
        }
//        err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Get);
        emit signals_cmd_result(cmd ,err);
        break;
    case CMD_PRN_PSaveTime_Set:
        err = protocol->cmd(VopProtocol::CMD_PRN_PSaveTime_Set);
        emit signals_cmd_result(cmd ,err);
        break;
    case CMD_PRN_PowerOff_Set:
        err = protocol->cmd(VopProtocol::CMD_PRN_PowerOff_Set);
        emit signals_cmd_result(cmd ,err);
        break;
    default:
        break;
    }
}
