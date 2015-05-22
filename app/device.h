#ifndef DEVICE_H
#define DEVICE_H

#include "vop_protocol.h"
class Device
{
public:
    Device();
    ~Device();

    int copy();
    void copy_set_parameter_default();

private:
    copycmdset* copy_parameter;
};

#endif // DEVICE_H
