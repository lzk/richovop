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

    connect(this ,SIGNAL(signals_cmd(int)) ,ctrl ,SLOT(slots_cmd(int)));
    connect(this ,SIGNAL(signals_progress(int)) ,widget ,SLOT(slots_progressBar(int)));

    connect(ctrl ,SIGNAL(signals_cmd_result(int,int)) ,this ,SIGNAL(signals_cmd_result(int ,int)));
    connect(this ,SIGNAL(signals_cmd_result(int,int)) ,widget ,SLOT(slots_cmd_result(int ,int)));

    connect(widget ,SIGNAL(signals_deviceChanged(QString)) ,this ,SIGNAL(signals_deviceChanged(QString)));
    connect(this ,SIGNAL(signals_deviceChanged(QString)) ,ctrl ,SLOT(slots_deviceChanged(QString)));

    app_block = new DeviceApp_Block(this);
    app_block->moveToThread(&app_block_thread);
    connect(app_block ,SIGNAL(signals_cmd(int)) ,ctrl ,SLOT(slots_cmd(int)));
    connect(this ,SIGNAL(signals_cmd_block(int)) ,app_block ,SLOT(slots_cmd(int)));

    deviceManageThread.start();
    app_block_thread.start();
}

DeviceApp::~DeviceApp()
{
    app_block_thread.quit();
    app_block_thread.wait();
    deviceManageThread.quit();
    deviceManageThread.wait();
    delete app_block;
    delete ctrl;
}

void DeviceApp::disconnect_App()
{
    ctrl->disconnect();
    widget->disconnect(this);
    disconnect();
}

bool DeviceApp::emit_cmd(int cmd)
{
//    bool ret = false;
    int status = get_cmdStatus();
    switch(status)
    {/*
    case DeviceContrl::CMD_STATUS_COMPLETE:    {
        if(DeviceContrl::CMD_COPY == cmd
                || DeviceContrl::CMD_WIFI_get == cmd
                || DeviceContrl::CMD_WIFI_apply == cmd
                || DeviceContrl::CMD_WIFI_getAplist == cmd
                ){
            emit signals_progress(0);
            emit signals_progress(20);
//            qLog()<<"show progress bar";
        }
        set_cmdStatus(cmd);
        emit signals_cmd(cmd);
        ret = true;
    }
        break;*/
    case DeviceContrl::CMD_PASSWD_confirmForApply:
    case DeviceContrl::CMD_WIFI_get://first get then get aplist
    case DeviceContrl::CMD_PASSWD_confirmForSetPasswd:
        if(DeviceContrl::CMD_WIFI_get == status){
            emit signals_progress(50);
//            qLog()<<"progress bar update";
        }
        set_cmdStatus(cmd);
        emit signals_cmd(cmd);
//        ret = true;
        break;

    default:
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
        emit signals_progress(100);
//        qLog()<<"cmd complete,hide progress bar";
    }
}

int DeviceApp::get_cmdStatus()
{
    QMutexLocker locker(&device_manager->mutex_app);
    return cmd_status;
}

void DeviceApp::emit_progress(int pro)
{
    emit signals_progress(pro);
}

#include <unistd.h>
void DeviceApp_Block::slots_cmd(int cmd)
{
    int status = app->get_cmdStatus();
    if(DeviceContrl::CMD_STATUS_COMPLETE != status){
        qLog()<<"waiting for cmd complete";
        do{
            usleep(100*1000);
            status = app->get_cmdStatus();
        }
        while(DeviceContrl::CMD_STATUS_COMPLETE != status);
    }
    if(DeviceContrl::CMD_COPY == cmd
            || DeviceContrl::CMD_WIFI_get == cmd
            || DeviceContrl::CMD_WIFI_apply == cmd
            || DeviceContrl::CMD_WIFI_getAplist == cmd
            ){
        emit app->emit_progress(0);
        emit app->emit_progress(20);
//            qLog()<<"show progress bar";
    }
    app->set_cmdStatus(cmd);
    emit signals_cmd(cmd);
}
