/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "vop_device.h"
#include "devicecontrol.h"
#include "log.h"
#include <unistd.h>

#ifndef DEVICE_LIB_SUPPORT
static int usbWriteThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    return -1;
}
static int usb_getDeviceID(const char* device_uri)
{
    return -1;
}
static int networkWriteThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    return -1;
}
static int net_getDeviceID(const char* device_uri)
{
    return -1;
}

#else
extern "C"{
#include "dlfcn.h"
int (*usb_openPrinter)(char*) = NULL;
void (*usb_closePrinter)(void) = NULL;
int (*usb_USBWrite)(char *buffer, size_t bufsize) = NULL;
int (*usb_USBRead)(char *buffer, size_t bufsize) = NULL;
int (*usb_get_device_id)(char *buffer, size_t bufsize) = NULL;

int (*net_connetToNetDevice)(char* devURI) = NULL;
int (*net_get_device_id)(char *buffer, size_t bufsize) = NULL;
int (*net_wirteToNetDevice)(void *buffer, int len) = NULL;
int (*net_readFromNetDevice)(void *buffer, int len) = NULL;
void (*net_closeSocket)(void) = NULL;
}
static int usbWriteThenRead(const char* device_uri ,char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = -1;
    if(usb_openPrinter)
        err = usb_openPrinter((char*)device_uri);
    else
        return -2;
    qLog()<<"usb_openPrinter:return"<<err;
    if(1 != err)
        return -1;

    int _write_size = 0,_read_size = 0;
    int i;
    for(i = 0 ;i < 3 ;i++){
#if 0 //copy from windows
        int sz = 256;
        char device_id[sz];
        memset(device_id ,0 ,sz);
        if(usb_get_device_id)
            err = usb_get_device_id(device_id ,sz);
        if(!err){
            char inBuffer[522] = { 0 };
            char outBuffer[12] = { 0 };
           memset(inBuffer, 0, sizeof(inBuffer));

           inBuffer[0] = 0x1B;
           inBuffer[1] = 0x4D;
           inBuffer[2] = 0x53;
           inBuffer[3] = 0x55;
           inBuffer[4] = 0xE0;
           inBuffer[5] = 0x2B;

           if(usb_USBWrite){
               usb_USBWrite(inBuffer ,10);
               usb_USBWrite(inBuffer + 10 ,512);
           }

           // acorrding the mail from Gerard:
           // " The reply of wakeup cmd is defined in Toolbox cmd spec,
           // 12 bytes in all, 1c 00 e0 2b  00 00 00 00  00 00 00 00
           // ". We read the "Print Bulk-in" package.

           if(usb_USBRead)
               _read_size = usb_USBRead(outBuffer , sizeof(outBuffer));
           char* buffer = outBuffer;
           qLog("reply of wakeup cmd:%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x"
                  ,buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],
                   buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10]);
        }
#endif
        err = 0;
        if(usb_USBWrite)
            _write_size = usb_USBWrite(wrBuffer ,wrSize);
        usleep(100 * 1000);//100 ms
        if(usb_USBRead)
            _read_size = usb_USBRead(rdBuffer ,rdSize);
        if((_write_size == wrSize) && (_read_size == rdSize)){
            qLog()<<"try times:"<<i+1;
            break;
        }
        err = -1;
    }

    if(usb_closePrinter)
        usb_closePrinter();
    else
        return -2;
    qLog()<<"usb_closePrinter";

    return err;
}

static int usb_getDeviceID(const char* device_uri ,char* buffer ,int buffer_size)
{
    int err = -1;
    if(usb_openPrinter)
        err = usb_openPrinter((char*)device_uri);
    else
        return -2;
//    qLog()<<"usb_openPrinter:return"<<err;
    if(1 != err)
        return -1;

    if(usb_get_device_id)
        err = usb_get_device_id(buffer ,buffer_size);
    else
        return -2;
//    qLog()<<"usb_get_device_id:"<<buffer<<"\nsize:"<<strlen(buffer);
    if(1 != err)
    {
        err = -1;
    }else
        err = 0;

    if(usb_closePrinter)
        usb_closePrinter();
    else
        return -2;
//        qLog()<<"usb_closePrinter";

    return err;
}

static int networkWriteThenRead(const char* device_uri ,char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = -1;
    if(net_connetToNetDevice)
        err = net_connetToNetDevice((char*)device_uri);
    else
        return -2;
    qLog()<<"net_connetToNetDevice:return"<<err;
    if(1 != err)
        return -1;

    int _write_size = 0,_read_size = 0;
    int i;
    for(i = 0 ;i < 3 ;i++){
        err = 0;
        if(net_wirteToNetDevice)
            _write_size = net_wirteToNetDevice(wrBuffer ,wrSize);
        usleep(100 * 1000);//100 ms
        if(net_readFromNetDevice)
            _read_size = net_readFromNetDevice(rdBuffer ,rdSize);

        qLog()<<"try times:"<<i+1<<"\twirte size:"<<wrSize<<":"<<_write_size<<"\tread size:"<<rdSize<<":"<<_read_size;
        if((_write_size == wrSize) && (_read_size == rdSize)){
            break;
        }
        err = -1;
    }
    if(net_closeSocket)
        net_closeSocket();
    else
        return -2;
    qLog()<<"net_closeSocket:";

    return err;
}

static int net_getDeviceID(const char* device_uri ,char* buffer ,int buffer_size)
{
    int err = -1;
    if(net_connetToNetDevice)
        err = net_connetToNetDevice((char*)device_uri);
    else
        return -2;
    qLog()<<"net_connetToNetDevice:return"<<err;
    if(1 != err)
        return -1;

    if(net_get_device_id)
        err = net_get_device_id(buffer ,buffer_size);
    else
        return -2;
    qLog()<<"net_get_device_id:return"<<err;

    if(net_closeSocket)
        net_closeSocket();
    else
        return -2;
    qLog()<<"net_closeSocket:";

    err = 0;//no err
    return err;
}
#endif


VopDevice::VopDevice()
    :hLLD_usb(NULL),
      hLLD_net(NULL)
{
    hLLD_usb = dlopen("./libvopusb.so", RTLD_LAZY);
    if(hLLD_usb)    {
        usb_openPrinter = (int (*)(char*))dlsym(hLLD_usb, "openPrinter");
        usb_closePrinter = (void (*)(void))dlsym(hLLD_usb, "closePrinter");
        usb_USBWrite = (int (*)(char *, size_t ))dlsym(hLLD_usb, "USBWrite");
        usb_USBRead = (int (*)(char *, size_t ))dlsym(hLLD_usb, "USBRead");
        usb_get_device_id = (int (*)(char *, size_t ))dlsym(hLLD_usb, "get_device_id");
    }else{
        qLog()<<"dlerror:"<<dlerror();
    }

    hLLD_net = dlopen("./libsocket.so", RTLD_LAZY);
    if(hLLD_net)    {
        net_connetToNetDevice = (int (*)(char*))dlsym(hLLD_net, "connetToNetDevice");
        net_closeSocket = (void (*)(void))dlsym(hLLD_net, "closeSocket");
        net_wirteToNetDevice = (int (*)(void *, int ))dlsym(hLLD_net, "wirteToNetDevice");
        net_readFromNetDevice = (int (*)(void *, int ))dlsym(hLLD_net, "readFromNetDevice");
        net_get_device_id = (int (*)(char *, size_t ))dlsym(hLLD_net, "get_device_id_net");
    }
}

VopDevice::~VopDevice()
{
    if(hLLD_usb)    {
        dlclose(hLLD_usb);
    }
    if(hLLD_net)    {
        dlclose(hLLD_net);
    }
}

int VopDevice::isValidDevice(const char* printer_info)
{
//    return true;
    int valid = false;
    QString str(printer_info);
    if(str.startsWith("Lenovo M7208W"))
    {
        valid = true;
    }else if(str.startsWith("Lenovo M7208")){
        valid = true;
    }else if(str.startsWith("Lenovo LJ2208W")){
        valid = true;
    }else if(str.startsWith("Lenovo LJ2208")){
        valid = true;
    }
    return valid;
}

bool VopDevice::is_usbDevice(const QString& str)
{
    return str.startsWith("usb://");
}

bool VopDevice::is_netDevice(const QString& str)
{
    return (str.startsWith("socket://")
            || str.startsWith("dnssd://")
//            || str.startsWith("lpd://")
//            || str.startsWith("ipp://")
//            || str.startsWith("mdns://")
            );
}

int VopDevice::writeThenRead(const char* device_uri ,char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = -1;
//    QString _device_uri = QString(device_uri);
//    qLog()<<__func__<<"device-uri"<<_device_uri;

//    return -1;

    if(is_usbDevice(device_uri)){
        err = usbWriteThenRead(device_uri ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    }else if(is_netDevice(device_uri)){
        err = networkWriteThenRead(device_uri ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    }else{
        qLog()<<"the uri is not correct";
    }

    return err;
}

int VopDevice::getDeviceStatus(const char* device_uri ,char* buffer ,int buffer_size)
{
    int err = -1;
//    QString _device_uri = QString(device_uri);

    if(is_usbDevice(device_uri)){
        err = usb_getDeviceID(device_uri ,buffer ,buffer_size);
    }else if(is_netDevice(device_uri)){
        err = net_getDeviceID(device_uri ,buffer ,buffer_size);
    }
    return err;
}

