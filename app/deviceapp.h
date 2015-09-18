/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef DEVICEAPP_H
#define DEVICEAPP_H

#include <QObject>
#include<QThread>

class DeviceContrl;
class DeviceManager;
class MainWidget;
class DeviceApp : public QObject
{
    Q_OBJECT
public:
    explicit DeviceApp(DeviceManager* dm ,MainWidget* widget);
    ~DeviceApp();

    void disconnect_App();

    bool get_passwd_confirmed();
    void set_passwd_confirmed(bool b);
    bool emit_cmd_plus(int);

signals:
    void signals_cmd_plus(int);
    void signals_progress(int,int);


private:
    QThread deviceManageThread;
    DeviceManager* device_manager;
    DeviceContrl* ctrl;
    MainWidget* main_widget;
};

#endif // DEVICE_H
