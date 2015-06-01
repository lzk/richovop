#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>

#include <cups/cups.h>
#include <QMutex>
#include <QStringList>

class VopDevice;
typedef  struct _copycmdset copycmdset;
class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = 0);
    ~DeviceManager();

    int getDeviceList(QStringList& printerInfo);
    void selectDevice(int selected_device);
    void setDefaultCopyParameter();
    void setCopyParameter(copycmdset* p);
    copycmdset getCopyParameter();
    const char* getCurrentDeviceURI();

    static int device_cmd(const char* device_name  ,char* buffer ,int len);

private:
    QMutex mutex;
    VopDevice* device;
    QStringList devices;
    QString selected_devicename;
    QString current_devicename;

    int isDeviceValid(const char* value);

signals:
    void signals_device_status(int);

public slots:
    void slots_device_status();
    void slots_copy();

};

#endif // DEVICEMANAGER_H
