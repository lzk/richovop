#include "usbdevice.h"
#include "log.h"
#include "vop_protocol.h"

#include <QApplication>
#include <string.h>
extern "C"{
#include <dlfcn.h>
}

static int (*hLLD_openPrinter)(char*) = NULL;
static void (*hLLD_closePrinter)(void) = NULL;
static int (*hLLD_write)(char *buffer, size_t bufsize) = NULL;
static int (*hLLD_read)(char *buffer, size_t bufsize) = NULL;
static int (*hLLD_get_device_id)(char *buffer, size_t bufsize) = NULL;
static int (*hLLD_get_device_status)(int busno, int dev_addr) = NULL;

UsbDevice::UsbDevice()
{
    QString path = QApplication::applicationDirPath();
    hLLD = dlopen( (path + "/libvopusb.so").toLatin1(), RTLD_LAZY);
    if(hLLD)    {
        hLLD_openPrinter  = (int (*)(char*))dlsym(hLLD, "openPrinter");
        hLLD_closePrinter  = (void (*)(void))dlsym(hLLD, "closePrinter");
        hLLD_write  = (int (*)(char *, size_t ))dlsym(hLLD, "USBWrite");
        hLLD_read  = (int (*)(char *, size_t ))dlsym(hLLD, "USBRead");
        hLLD_get_device_id  = (int (*)(char *, size_t ))dlsym(hLLD, "get_device_id");
        hLLD_get_device_status = (int (*)(int ,int))dlsym(hLLD ,"get_device_status");
    }else{
        _Q_LOG("can not open usb backend.");
        _Q_LOG(QString("dlerror:") + dlerror());
    }
}

UsbDevice::~UsbDevice()
{
}

void UsbDevice::init(char*)
{

}

int UsbDevice::openPrinter(const char* device_uri)
{
    if(!hLLD_openPrinter)
        return ERR_library;
    int err = hLLD_openPrinter((char*)device_uri);
    if(1 != err){
        _Q_LOG("can not open usb printer");
        err = ERR_communication;
    }else
        err = ERR_ACK;
    return err;
}

int UsbDevice::openPrinter(void)
{

    return ERR_ACK;
}

int UsbDevice::closePrinter(void)
{
    if(!hLLD_closePrinter)
        return ERR_library;
    hLLD_closePrinter();
    return ERR_ACK;
}

int UsbDevice::write(char *buffer, int bufsize)
{
    if(!hLLD_write)
        return ERR_library;
    return hLLD_write(buffer ,bufsize);
}

int UsbDevice::read(char *buffer, int bufsize)
{
    if(!hLLD_read)
        return ERR_library;
    return hLLD_read(buffer ,bufsize);
}

int UsbDevice::get_device_id(char *buffer, int bufsize)
{
    if(!hLLD_get_device_id)
        return ERR_library;
    int err = hLLD_get_device_id(buffer ,bufsize);
    if(1 != err){
        _Q_LOG("can not get device id");
        err = ERR_communication;
    }else
        err = ERR_ACK;
    return err;
}

int UsbDevice::get_driver()
{

    return ERR_ACK;
}

int UsbDevice::attach_driver()
{

    return ERR_ACK;
}

#include "linux_api.h"
#include <QSettings>
bool UsbDevice::is_device_scanning()
{
    if(!scanner_locked())
        return false;
    if(!hLLD_get_device_status)
        return true;

    int bus_NO , device_address;
    QSettings settings("/tmp/.alto_used",QSettings::NativeFormat);
    bus_NO = settings.value("Bus_Number" ,0).toInt();
    device_address = settings.value("Device_Address" ,0).toInt();
    if(!bus_NO || !device_address)
        return false;
    if(hLLD_get_device_status(bus_NO ,device_address))
        return true;
    return false;
}

bool UsbDevice::is_Airprint_scanning()
{
    if(!airprint_scanner_locked())
        return false;
    if(!hLLD_get_device_status)
        return true;

    int bus_NO , device_address;
    QSettings settings("/tmp/.alto_airprint_used",QSettings::NativeFormat);
    bus_NO = settings.value("Bus_Number" ,0).toInt();
    device_address = settings.value("Device_Address" ,0).toInt();
    if(!bus_NO || !device_address)
        return false;
    if(hLLD_get_device_status(bus_NO ,device_address))
        return true;
    return false;
}
