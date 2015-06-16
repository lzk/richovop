#include "deviceapp.h"

#include "devicemanager.h"
#include "../mainwidget.h"
DeviceApp::DeviceApp(QObject *parent) :
    QObject(parent),
    deviceManager (new DeviceManager),
    cmd_status(DeviceManager::CMD_STATUS_COMPLETE)
{
    deviceManager->moveToThread(&deviceManageThread);
    connect(this ,SIGNAL(signals_cmd(int)) ,deviceManager ,SLOT(slots_cmd(int)));
    MainWidget* widget = qobject_cast<MainWidget*>(parent);
    connect(deviceManager ,SIGNAL(signals_cmd_result(int,int)) ,this ,SIGNAL(signals_cmd_result(int ,int)));
    connect(this ,SIGNAL(signals_cmd_result(int,int)) ,widget ,SLOT(slots_cmd_result(int ,int)));

    connect(deviceManager ,SIGNAL(signals_setProgress(int)) ,this ,SLOT(signals_progress(int)));
    connect(this ,SIGNAL(signals_progress(int)) ,widget ,SLOT(slots_progressBar(int)));

    deviceManageThread.start();
}

DeviceApp::~DeviceApp()
{
    deviceManageThread.quit();
    deviceManageThread.wait();
    delete deviceManager;
}


bool DeviceApp::emit_cmd(int cmd)
{
    bool ret = false;
    switch(cmd_status)
    {
    case DeviceManager::CMD_STATUS_COMPLETE:
    case DeviceManager::CMD_PASSWD_confirmForApply:
    case DeviceManager::CMD_PASSWD_confirmForSetPasswd:
    case DeviceManager::CMD_WIFI_get://first get then get aplist
        cmd_status = cmd;
        emit signals_cmd(cmd);
        ret = true;
        break;

    default:
        break;
    }
    return ret;
}

#if 0
void DeviceApp::slots_progress(int value)
{
//    progressDialog->setValue(value);
}

void DeviceApp::slots_emit_cmd(int cmd)
{
    int status = cmd_status;
    while(DeviceManager::CMD_STATUS_COMPLETE != status){
        status = cmd_status;
        sleep(1);
    }
//    if(DeviceManager::CMD_STATUS_COMPLETE == cmd_status){
        cmd_status = cmd;
        emit signals_cmd(cmd);
//    }
}

void DeviceApp::slots_cmd_result(int cmd ,int err)
{
    switch(cmd)
    {
    case DeviceManager::CMD_DEVICE_status:
        break;
    case DeviceManager::CMD_WIFI_get:
        break;
    case DeviceManager::CMD_PASSWD_confirmForApply:
        if(!err){//no err,ACK
            passwd_checked = true;

            cmdst_wifi_get wifi_para = deviceManager->wifi_get_para();
            //setting data then apply
            deviceManager->wifi_set_password(&wifi_para ,wifi_password.toLatin1());
            deviceManager->wifi_set_ssid(&wifi_para ,wifi_ssid.toLatin1());
            wifi_para.encryption = wifi_encryptionType > 1 ? wifi_encryptionType + 1 :wifi_encryptionType;
            wifi_para.wepKeyId = wifi_wepIndex;
            wifi_para.wifiEnable &= ~1;
            wifi_para.wifiEnable |= ts->checkBox->isChecked() ? 1 : 0;//bit 0
            deviceManager->wifi_set_para(&wifi_para);
            emit_cmd(DeviceManager::CMD_WIFI_apply);
        }else if(ERR_Password_incorrect == err){//password incorrect
            passwd_checked = false;
        }
        break;
    case DeviceManager::CMD_PASSWD_confirmForSetPasswd:
        if(!err){//no err,ACK
            deviceManager->passwd_set(ts->le_newPassword->text().toLatin1());
            emit_cmd(DeviceManager::CMD_PASSWD_set);
        }else if(ERR_Password_incorrect == err){//password incorrect
            passwd_checked = false;
        }
        break;
    case DeviceManager::CMD_WIFI_getAplist:
        break;
    default:
        break;
    }
    cmd_status = DeviceManager::CMD_STATUS_COMPLETE;
}

#endif
