#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include<QThread>

class DeviceManager;
class DeviceApp : public QObject
{
    Q_OBJECT
public:
    explicit DeviceApp(QObject *parent = 0);
    ~DeviceApp();

    bool emit_cmd(int);
public:
    DeviceManager* deviceManager;

signals:
    void signals_cmd(int);
    void signals_cmd_result(int,int);
    void signals_progress(int);

public slots:
//    void slots_emit_cmd(int);
//    void slots_cmd_result(int ,int);
//    void slots_progress(int);

private:
    QThread deviceManageThread;
    int cmd_status;
};

#endif // DEVICE_H
