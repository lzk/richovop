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
class DeviceApp ;
class DeviceApp_Block:public QObject
{
    Q_OBJECT
public:
    explicit DeviceApp_Block(DeviceApp* _app):app(_app){}
    ~DeviceApp_Block(){}
private:
    DeviceApp* app;

signals:
    void signals_cmd(int);
public slots:
    void slots_cmd(int);
};

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
    void emit_progress(int);

signals:
    void signals_cmd(int);
//    void signals_cmd_result(int,int);
    void signals_progress(int);
//    void signals_deviceChanged(QString);
    void signals_cmd_block(int);

public slots:
//    void slots_emit_cmd(int);

private:
    QThread deviceManageThread;
    DeviceManager* device_manager;
    DeviceContrl* ctrl;
    int cmd_status;
    MainWidget* widget;
    DeviceApp_Block* app_block;
    QThread app_block_thread;
};

#endif // DEVICE_H
