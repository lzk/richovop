/////////////////////////////////////////
/// File:vop_device.h
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef DEVICE_H
#define DEVICE_H

#include <stddef.h>
class DeviceManager;
class VopDevice
{
public:
    VopDevice(DeviceManager* dm);
    ~VopDevice();

    static int isValidDevice(const char* printer_info);
    static int writeThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize);
    static int getDeviceStatus(const char* device_uri);

private:
    void* hLLD_usb;
    void* hLLD_net;
    DeviceManager* deviceManager;
};

#endif // DEVICE_H
