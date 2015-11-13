#include "usbdevice.h"
#include "log.h"
#include "vop_protocol.h"

#include <QApplication>
#include <string.h>
extern "C"{
#include <dlfcn.h>

/* some systems do not have newest memcpy@@GLIBC_2.14 - stay with old good one */
#ifdef __x86_64__
asm (".symver memcpy, memcpy@GLIBC_2.2.5");
#elif __i386__
asm (".symver memcpy, memcpy@GLIBC_2.0");
#endif

//void *memcpy(void* ,const void* ,size_t);
void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

}

static int (*hLLD_openPrinter)(char*) = NULL;
static void (*hLLD_closePrinter)(void) = NULL;
static int (*hLLD_write)(char *buffer, size_t bufsize) = NULL;
static int (*hLLD_read)(char *buffer, size_t bufsize) = NULL;
static int (*hLLD_get_device_id)(char *buffer, size_t bufsize) = NULL;

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
    }else{
        qLog("can not open usb backend.");
        qLog(QString("dlerror:") + dlerror());
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
        qLog("can not open printer");
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
        qLog("can not get device id");
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
