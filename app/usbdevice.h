#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "device.h"
class UsbDevice : public Device
{
public:
    UsbDevice();
    ~UsbDevice();

    virtual  void init(char*);
    virtual  int openPrinter(const char*);
    virtual  int openPrinter(void);
    virtual  int closePrinter(void);
    virtual  int write(char *buffer, int bufsize);
    virtual  int read(char *buffer, int bufsize);
    virtual  int get_device_id(char *buffer, int bufsize);
    virtual  int get_driver();
    virtual  int attach_driver();

    static bool is_device_scanning();
};

#endif // USBDEVICE_H
