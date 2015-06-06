#ifndef DEVICE_H
#define DEVICE_H

#include "vop_protocol.h"

class VopCopy
{
public:
    VopCopy();
    ~VopCopy();

    void set_parameter_default();
    void setCopyParameter(copycmdset* p);
    copycmdset getCopyParameter();
private:
    copycmdset* copy_parameter;
};

class VopDevice
{
public:
    VopDevice();
    ~VopDevice();

    VopCopy* getVopCopy();
    static int isValidDevice(const char* printer_info);
    static int writeThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize);
private:
    VopCopy* vopCopy;
};

#endif // DEVICE_H
