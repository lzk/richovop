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
    device(dm->device),
    protocol(dm->protocol)
//    device_manager(dm)
{    
    device = dm->device;
    protocol = dm->protocol;
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
//    device_uri = "socket://192.168.6.123:9100";
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
    case CMD_DEVICE_status:    {
        err = protocol->cmd(VopProtocol::CMD_GetStatus);
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

