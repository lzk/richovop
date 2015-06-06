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

    static QString current_devicename;

    int getDeviceList(QStringList& printerInfo);
    void selectDevice(int selected_device);
    void setDefaultCopyParameter();
    void setCopyParameter(copycmdset* p);
    copycmdset getCopyParameter();
    QString getCurrentDeviceURI();

    static QString getDeviceURI(const QString&);
    static int device_writeThenRead(const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize);

private:
    QMutex mutex;
    VopDevice* device;
    QStringList devices;
    QString selected_devicename;

signals:
    void signals_device_status(int);

public slots:
    void slots_device_status();
    void slots_copy();

};

#endif // DEVICEMANAGER_H
