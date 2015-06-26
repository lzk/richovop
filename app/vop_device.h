/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef VOP_DEVICE_H
#define VOP_DEVICE_H

#include <stddef.h>
class DeviceContrl;
#include<QString>
class VopDevice
{
public:
    VopDevice();
    ~VopDevice();

    static bool is_usbDevice(const QString&);
    static bool is_netDevice(const QString&);
    static int isValidDevice(const char* printer_info);    
    static int writeThenRead(const char* device_uri ,char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize);
    static int getDeviceStatus(const char* device_uri ,char* buffer ,int buffer_size);

private:
    void* hLLD_usb;
    void* hLLD_net;
};

#endif // VOP_DEVICE_H
