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
    main_widget(_widget)
{
    ctrl->moveToThread(&deviceManageThread);
    connect(&deviceManageThread, SIGNAL(finished()), ctrl, SLOT(deleteLater()));
    connect(this ,SIGNAL(signals_cmd_plus(int)) ,ctrl ,SLOT(slots_cmd_plus(int)));
    connect(this ,SIGNAL(signals_progress(int ,int)) ,main_widget ,SLOT(slots_progressBar(int ,int)));
    connect(ctrl ,SIGNAL(signals_progress(int ,int)) ,main_widget ,SLOT(slots_progressBar(int ,int)));
    connect(ctrl ,SIGNAL(signals_cmd_result(int,int))  ,main_widget ,SLOT(slots_cmd_result(int ,int)));
    connect(main_widget ,SIGNAL(signals_deviceChanged(QString)) ,ctrl ,SLOT(slots_deviceChanged(QString)));

    deviceManageThread.start();
}

DeviceApp::~DeviceApp()
{
    deviceManageThread.quit();
    deviceManageThread.wait();
}

void DeviceApp::disconnect_App()
{
    ctrl->disconnect();
    main_widget->disconnect(this);
    disconnect();
}

bool DeviceApp::emit_cmd_plus(int cmd)
{
    emit signals_progress(cmd ,0);
    emit signals_progress(cmd ,20);
    emit signals_cmd_plus(cmd);
    return true;
}

bool DeviceApp::get_passwd_confirmed()
{
    return ctrl->get_passwd_confirmed();
}

void DeviceApp::set_passwd_confirmed(bool b)
{
        ctrl->set_passwd_confirmed(b);
}
