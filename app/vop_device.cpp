#include "vop_device.h"
#include <string.h>

static const copycmdset default_copy_parameter =
{
//    .Density   = 3,
//    .copyNum   = 1,
//    .scale     = 100

    3,//UINT8 Density         ; // 0  -   0~6
    1,//UINT8 copyNum         ; // 1  -   1~99
    0,//UINT8 scanMode        ; // 2  -   0: Photo, 1: Text, 2: ID card
    1,//UINT8 orgSize         ; // 3  -   0: A4, 1: A5, 2: B5, 3: Letter, 4: Executive
    1,//UINT8 paperSize       ; // 4  -   0: Letter, 1: A4, 2: A5, 3: A6, 4: B5, 5: B6, 6: Executive, 7: 16K
    0,//UINT8 nUp             ; // 5  -   0:1up, 1: 2up, 3: 4up, 4: 9up
    0,//UINT8 dpi             ; // 6  -   0: 300*300, 1: 600*600
    0,//UINT8 mediaType       ; // 7  -   0: plain paper 1: Recycled paper 2: Thick paper 3: Thin paper 4: Label
    100,//UINT16 scale          ; // 8  -   25~400, disabled for 2/4/9up
};

VopCopy::VopCopy()
    : copy_parameter(new copycmdset)
{
    set_parameter_default();
}

VopCopy::~VopCopy()
{
    if(copy_parameter)
        delete copy_parameter;
}

void VopCopy::set_parameter_default()
{
    memcpy(copy_parameter ,&default_copy_parameter ,sizeof(default_copy_parameter));
}

void VopCopy::setCopyParameter(copycmdset* p)
{
    memcpy(copy_parameter ,p ,sizeof(copycmdset));
}

copycmdset VopCopy::getCopyParameter()
{
    return *copy_parameter;
}

VopDevice::VopDevice()
    : vopCopy(new VopCopy)
{
}

VopDevice::~VopDevice()
{
    if(vopCopy)
        delete vopCopy;
}

VopCopy* VopDevice::getVopCopy()
{
    return vopCopy;
}
