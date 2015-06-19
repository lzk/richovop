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

    bool emit_cmd(int);
    void set_cmdStatus(int);
    int get_cmdStatus();
public:

signals:
    void signals_cmd(int);
    void signals_cmd_result(int,int);
    void signals_progress(int);
    void signals_deviceChanged(QString);

public slots:
//    void slots_emit_cmd(int);

private:
    QThread deviceManageThread;
    DeviceManager* device_manager;
    DeviceContrl* ctrl;
    int cmd_status;
    MainWidget* widget;
};

#endif // DEVICE_H
