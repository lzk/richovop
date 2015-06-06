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

//////////////////////////VopDevice////////////////////////////
/// \brief VopDevice::VopDevice
///
//#include <cups/cups.h>
#include <QtCore>
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

int VopDevice::isValidDevice(const char* printer_info)
{
    int valid = false;
    QString str(printer_info);
    if(str.startsWith("Lenovo M7208W"))
    {
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

extern "C"{
//#include "lib/usb.h"
int openPrinter(char* printerURI);
void closePrinter(void);
int USBWrite(char *buffer, size_t bufsize);
int USBRead(char *buffer, size_t bufsize);
int get_device_id(char *buffer, size_t bufsize);
//#include "lib/NetDevice.h"
int connetToNetDevice(char* devURI);
int get_device_id(char *buffer, size_t bufsize);
int wirteToNetDevice(void *buffer, int len);
int readFromNetDevice(void *buffer, int len);
void closeSocket(void);
}
static int usbWriteThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = openPrinter((char*)device_uri);
    if(err)        return err;
    err = USBWrite((char*)wrBuffer ,wrSize);
    if(err)         goto ERR;
    err = USBRead(rdBuffer ,rdSize);
    if(err)         goto ERR;
    ERR:
    closePrinter();
    return err;
}

static int networkWriteThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = connetToNetDevice((char*)device_uri);
    if(err)        return err;
    err = wirteToNetDevice((char*)wrBuffer ,wrSize);
    if(err)         goto ERR;
    err = readFromNetDevice(rdBuffer ,rdSize);
    if(err)         goto ERR;
    ERR:
    closeSocket();
    return err;
}

int VopDevice::writeThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    return -1;
    int err = -1;
    QString _device_uri = QString(device_uri);
    if(_device_uri.startsWith("usb://"))
    {
        usbWriteThenRead(device_uri ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    }else{
        networkWriteThenRead(device_uri ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    }
    return err;
}
