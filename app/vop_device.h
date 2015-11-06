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

    enum{
        Device_invalid = 0,
        Device_3in1 = 1,
        Device_3in1_wifi = 2,
        Device_sfp = 3,
        Device_sfp_wifi = 4,

    };
    static bool is_usbDevice(const QString&);
    static bool is_netDevice(const QString&);
    static int getDeviceModel(const char* printer_info);
    static int writeThenRead(const char* device_uri ,char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize);
    static int getDeviceStatus(const char* device_uri ,char* buffer ,int buffer_size);
    static int open(const char* device_uri);
    static void close(const char* device_uri);

private:
    void* hLLD_usb;
    void* hLLD_net;
};

#endif // VOP_DEVICE_H
