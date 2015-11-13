#ifndef NETDEVICE_H
#define NETDEVICE_H

#include "device.h"
class NetDevice : public Device
{
public:
    NetDevice();
    ~NetDevice();

    virtual  void init(char*);
    virtual  int openPrinter(const char*);
    virtual  int openPrinter(void);
    virtual  int closePrinter(void);
    virtual  int write(char *buffer, int bufsize);
    virtual  int read(char *buffer, int bufsize);
    virtual  int get_device_id(char *buffer, int bufsize);
    virtual  int get_driver();
    virtual  int attach_driver();
    virtual int write_no_read(char* wrBuffer ,int wrSize);

};

#endif // NETDEVICE_H
