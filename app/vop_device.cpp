/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "vop_device.h"
#include "devicecontrol.h"
#include "log.h"
#include <unistd.h>

extern "C"{
#include "dlfcn.h"
}

struct device_control
{
    int (*openPrinter)(char*);
    void (*closePrinter)(void);
    int (*write)(char *buffer, size_t bufsize);
    int (*read)(char *buffer, size_t bufsize);
    int (*get_device_id)(char *buffer, size_t bufsize);
};
struct device_control usb_dc ={
  NULL,NULL,NULL,NULL,NULL
};
struct device_control net_dc ={
  NULL,NULL,NULL,NULL,NULL
};
static int write_then_read(struct device_control* dc ,const char* device_uri ,char* wrBuffer ,int wrSize ,char* rdBuffer, int rdSize)
{
    if(dc->openPrinter && dc->write && dc->read &&dc->closePrinter){
    }else{
        return -2;
    }
    if(1 != dc->openPrinter((char*)device_uri)){
        qLog()<<"can not open printer";
        return -1;
    }

    int err = -1;
    int _write_size = 0,_read_size = 0;
    int i;
    char writeBuffer[wrSize];
    memcpy(writeBuffer ,wrBuffer ,wrSize);
    for(i = 0 ;i < 3 ;i++){
        _write_size = dc->write(writeBuffer ,wrSize);
        usleep(100 * 1000);//100 ms
        if(_write_size != wrSize){
            //read all buffer last time?
            _read_size = dc->read(rdBuffer ,rdSize);
            continue;
        }

        int nocheck=0;
        for(int j = 0 ;j < 5 ;j++){
            if(!nocheck){
                if(1 == dc->read(rdBuffer,1)){
//                    qLog()<<QString().sprintf("waiting for 0x4d:%#.2x" ,rdBuffer[0]);
                    if(0x4d != rdBuffer[0]){
                        continue;
                    }
                }else{
                    continue;
                }
            }
            nocheck = 0;
            if(1 == dc->read(rdBuffer+1,1)){
//                qLog()<<QString().sprintf("waiting for 0x3c:%#.2x" ,rdBuffer[1]);
                if(0x3c == rdBuffer[1]){
                    _read_size = dc->read(rdBuffer+2 ,rdSize-2);
                    break;
                }else if(0x4d == rdBuffer[1]){
                    nocheck = 1;
                }
            }
        }
        if(_read_size == rdSize -2){
            err = 0;
            i++;
            break;
        }
    }
//    qLog()<<"try times:"<<i<<"\twirte size:"<<wrSize<<":"<<_write_size<<"\tread size:"<<rdSize<<":"<<_read_size+2;
    dc->closePrinter();
    return err;
}
static int get_deviceID(struct device_control* dc ,const char* device_uri ,char* buffer ,int buffer_size)
{
    if(dc->openPrinter && dc->get_device_id && dc->closePrinter){
    }else{
        return -2;
    }
    if(1 != dc->openPrinter((char*)device_uri)){
        qLog()<<"can not open usb printer";
        return -1;
    }
    int err = -1;
    err = dc->get_device_id(buffer ,buffer_size);
//    qLog()<<"usb_get_device_id:"<<buffer<<"\nsize:"<<strlen(buffer);
    if(1 != err){
        err = -1;
    }else
        err = 0;
    dc->closePrinter();
//        qLog()<<"usb_closePrinter";
    return err;
}

VopDevice::VopDevice()
    :hLLD_usb(NULL),
      hLLD_net(NULL)
{
    hLLD_usb = dlopen("./libvopusb.so", RTLD_LAZY);
    if(hLLD_usb)    {
        usb_dc.openPrinter  = (int (*)(char*))dlsym(hLLD_usb, "openPrinter");
        usb_dc.closePrinter  = (void (*)(void))dlsym(hLLD_usb, "closePrinter");
        usb_dc.write  = (int (*)(char *, size_t ))dlsym(hLLD_usb, "USBWrite");
        usb_dc.read  = (int (*)(char *, size_t ))dlsym(hLLD_usb, "USBRead");
        usb_dc.get_device_id  = (int (*)(char *, size_t ))dlsym(hLLD_usb, "get_device_id");
    }else{
        qLog()<<"dlerror:"<<dlerror();
    }

    hLLD_net = dlopen("./libsocket.so", RTLD_LAZY);
    if(hLLD_net)    {
        net_dc.openPrinter  = (int (*)(char*))dlsym(hLLD_net, "connetToNetDevice");
        net_dc.closePrinter  = (void (*)(void))dlsym(hLLD_net, "closeSocket");
        net_dc.write  = (int (*)(char *, size_t ))dlsym(hLLD_net, "wirteToNetDevice");
        net_dc.read  = (int (*)(char *, size_t ))dlsym(hLLD_net, "readFromNetDevice");
        net_dc.get_device_id  = (int (*)(char *, size_t ))dlsym(hLLD_net, "get_device_id_net");
    }else{
        qLog()<<"dlerror:"<<dlerror();
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
    if(str.startsWith("Lenovo M7208W"))    {
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
    if(is_usbDevice(device_uri)){
        err = write_then_read(&usb_dc ,device_uri ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    }else if(is_netDevice(device_uri)){
        err = write_then_read(&net_dc ,device_uri ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    }else{
        qLog()<<"the uri is not correct";
    }
    return err;
}

int VopDevice::getDeviceStatus(const char* device_uri ,char* buffer ,int buffer_size)
{
    int err = -1;
    if(is_usbDevice(device_uri)){
        err = get_deviceID(&usb_dc ,device_uri ,buffer ,buffer_size);
    }else if(is_netDevice(device_uri)){
        err = get_deviceID(&net_dc ,device_uri ,buffer ,buffer_size);
    }else{
        qLog()<<"the uri is not correct";
    }
    return err;
}

