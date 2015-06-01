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
private:
    VopCopy* vopCopy;
};

#endif // DEVICE_H
