/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "deviceapp.h"

#include "devicecontrol.h"
#include "devicemanager.h"
#include "../mainwidget.h"
#include "log.h"
DeviceApp::DeviceApp(DeviceManager* dm ,MainWidget* _widget) :
    device_manager(dm),
    ctrl (new DeviceContrl(dm)),
    cmd_status(DeviceContrl::CMD_STATUS_COMPLETE),
    widget(_widget)
{
    ctrl->moveToThread(&deviceManageThread);
    connect(&deviceManageThread, SIGNAL(finished()), ctrl, SLOT(deleteLater()));

    connect(this ,SIGNAL(signals_cmd(int)) ,ctrl ,SLOT(slots_cmd(int)));
    connect(this ,SIGNAL(signals_progress(int ,int)) ,widget ,SLOT(slots_progressBar(int ,int)));

//    connect(ctrl ,SIGNAL(signals_cmd_result(int,int)) ,this ,SIGNAL(signals_cmd_result(int ,int)));
//    connect(this ,SIGNAL(signals_cmd_result(int,int)) ,widget ,SLOT(slots_cmd_result(int ,int)));
    connect(ctrl ,SIGNAL(signals_cmd_result(int,int))  ,widget ,SLOT(slots_cmd_result(int ,int)));

//    connect(widget ,SIGNAL(signals_deviceChanged(QString)) ,this ,SIGNAL(signals_deviceChanged(QString)));
//    connect(this ,SIGNAL(signals_deviceChanged(QString)) ,ctrl ,SLOT(slots_deviceChanged(QString)));
    connect(widget ,SIGNAL(signals_deviceChanged(QString)) ,ctrl ,SLOT(slots_deviceChanged(QString)));

    app_block = new DeviceApp_Block(this);
    app_block->moveToThread(&app_block_thread);
    connect(&app_block_thread, SIGNAL(finished()), app_block, SLOT(deleteLater()));
    connect(app_block ,SIGNAL(signals_cmd(int)) ,ctrl ,SLOT(slots_cmd(int)));
    connect(this ,SIGNAL(signals_cmd_block(int)) ,app_block ,SLOT(slots_cmd(int)));

    deviceManageThread.start();
    app_block_thread.start();
}

DeviceApp::~DeviceApp()
{
    deviceManageThread.quit();
    deviceManageThread.wait();
    app_block_thread.quit();
    app_block_thread.wait();
}

void DeviceApp::disconnect_App()
{
    ctrl->disconnect();
    widget->disconnect(this);
    disconnect();
}
//static int progress = 0;
bool DeviceApp::emit_cmd(int cmd)
{
//    bool ret = false;
    switch(cmd)
    {
    case DeviceContrl::CMD_PASSWD_set:
    case DeviceContrl::CMD_WIFI_getAplist:
    case DeviceContrl::CMD_WIFI_GetWifiStatus_immediately:
    case DeviceContrl::CMD_WIFI_apply:
//        qLog("cmd continue immeditaly");
        set_cmdStatus(cmd);
        emit signals_cmd(cmd);
//        if(progress <= 50)
//            progress += 30;
//        emit emit_progress(progress);
        emit emit_progress(cmd ,50);
        break;
    case DeviceContrl::CMD_DEVICE_status://last cmd is get device id
    default:
//        qLog("cmd start block");
        emit signals_cmd_block(cmd);
        break;
    }
//    return ret;
    return true;
}

void DeviceApp::set_cmdStatus(int status)
{
    QMutexLocker locker(&device_manager->mutex_app);
    cmd_status = status;
    if(DeviceContrl::CMD_STATUS_COMPLETE == status)
    {
        //if progress bar display,hide it
        emit signals_progress(status ,100);
//        qLog("cmd complete,hide progress bar");
    }
}

int DeviceApp::get_cmdStatus()
{
    QMutexLocker locker(&device_manager->mutex_app);
    return cmd_status;
}

void DeviceApp::emit_progress(int cmd ,int pro)
{
    emit signals_progress(cmd ,pro);
}

#include <unistd.h>
void DeviceApp_Block::slots_cmd(int cmd)
{
    int status = app->get_cmdStatus();
    if(DeviceContrl::CMD_STATUS_COMPLETE != status){
        qLog("waiting for cmd complete");
        do{
            usleep(100*1000);
            status = app->get_cmdStatus();
        }
        while(DeviceContrl::CMD_STATUS_COMPLETE != status);
    }
    if(cmd != DeviceContrl::CMD_DEVICE_status){
        emit app->emit_progress(cmd ,0);
        emit app->emit_progress(cmd ,20);
//        progress = 20;
    }
    app->set_cmdStatus(cmd);
    emit signals_cmd(cmd);
}
