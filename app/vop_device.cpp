/////////////////////////////////////////
/// File:vop_device.cpp
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "vop_device.h"
#include "devicemanager.h"
#include <QDebug>
#include <QtCore>

#if 1
//#include <cups/cups.h>
#include<cups/sidechannel.h>
#include<cups/backend.h>
#include <string.h>
#include <unistd.h>
int device_cmd(char* buffer ,int len)
{
    int result = 0;
    (void)buffer;
    (void)len;
    return result ;

    char data[2049];
    int datalen = 2048;
    cups_sc_status_t status =
    cupsSideChannelDoRequest(CUPS_SC_CMD_GET_DEVICE_ID ,data ,&datalen ,1.0);
    if(status == CUPS_SC_STATUS_OK && datalen > 0)
        data[datalen] = '\0';
    else
        data[0]='\0';
    char* s = data;
    qDebug()<<"device id"<<s;
}

void exec_backend(char **argv)		/* I - Command-line arguments */
{
  const char	*resolved_uri,		/* Resolved device URI */
        *cups_serverbin;	/* Location of programs */
  char		scheme[1024],		/* Scheme from URI */
        *ptr,			/* Pointer into scheme */
        filename[1024];		/* Backend filename */


 /*
  * Resolve the device URI...
  */

  while ((resolved_uri = cupsBackendDeviceURI(argv)) == NULL)
  {
//    _cupsLangPrintFilter(stderr, "INFO", _("Unable to locate printer."));
    qDebug()<<"Unable to locate printer.";
    sleep(10);

    if (getenv("CLASS") != NULL)
      exit(CUPS_BACKEND_FAILED);
  }

 /*
  * Extract the scheme from the URI...
  */

//  strlcpy(scheme, resolved_uri, sizeof(scheme));
  if ((ptr = strchr(scheme, ':')) != NULL)
    *ptr = '\0';

 /*
  * Get the filename of the backend...
  */

  if ((cups_serverbin = getenv("CUPS_SERVERBIN")) == NULL)
//    cups_serverbin = CUPS_SERVERBIN;

  snprintf(filename, sizeof(filename), "%s/backend/%s", cups_serverbin, scheme);

 /*
  * Overwrite the device URI and run the new backend...
  */

  setenv("DEVICE_URI", resolved_uri, 1);

  argv[0] = (char *)resolved_uri;

  fprintf(stderr, "DEBUG: Executing backend \"%s\"...\n", filename);

  execv(filename, argv);

//  fprintf(stderr, "ERROR: Unable to execute backend \"%s\": %s\n", filename,
//          strerror(errno));
//  exit(CUPS_BACKEND_STOP);
}
#endif

#ifndef DEVICE_LIB_SUPPORT
static int usbWriteThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    return -1;
}
static int usb_getDeviceID(const char* device_uri)
{
    return -1;
}
static int networkWriteThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    return -1;
}
static int net_getDeviceID(const char* device_uri)
{
    return -1;
}

#else
extern "C"{
#include "dlfcn.h"
int (*usb_openPrinter)(char*) = NULL;
void (*usb_closePrinter)(void) = NULL;
int (*usb_USBWrite)(char *buffer, size_t bufsize) = NULL;
int (*usb_USBRead)(char *buffer, size_t bufsize) = NULL;
int (*usb_get_device_id)(char *buffer, size_t bufsize) = NULL;

int (*net_connetToNetDevice)(char* devURI) = NULL;
int (*net_get_device_id)(char *buffer, size_t bufsize) = NULL;
int (*net_wirteToNetDevice)(void *buffer, int len) = NULL;
int (*net_readFromNetDevice)(void *buffer, int len) = NULL;
void (*net_closeSocket)(void) = NULL;
}
static int usbWriteThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = -1;
    if(usb_openPrinter)
        err = usb_openPrinter((char*)device_uri);
    else
        return -2;
    qDebug()<<"usb_openPrinter:return"<<err;
//    if(err)        return err;

    if(usb_USBWrite)
        err = usb_USBWrite((char*)wrBuffer ,wrSize);
    else
        return -2;
    qDebug()<<"usb_USBWrite:return"<<err;
//    if(wrSize != err)
//    {
//        err = -1;
//        goto ERR;
//    }else
//        err = 0;

    if(usb_USBRead)
        err = usb_USBRead(rdBuffer ,rdSize);
    else
        return -2;
    qDebug()<<"usb_USBRead:return"<<err;
//    if(rdSize != err)
//    {
//        err = -1;
//        goto ERR;
//    }else
//        err = 0;
ERR:
    if(usb_closePrinter)
        usb_closePrinter();
    else
        return -2;
    qDebug()<<"usb_closePrinter";
    return err;
}
static int usb_getDeviceID(const char* device_uri)
{
    char device_id[256];
    int err = -1;
    if(usb_openPrinter)
        err = usb_openPrinter((char*)device_uri);
    else
        return -2;
    qDebug()<<"usb_openPrinter:return"<<err;
//    if(err)        return err;

    if(usb_get_device_id)
        err = usb_get_device_id(device_id ,256);
    else
        return -2;
    qDebug()<<"usb_get_device_id:return"<<err;
//    if(err)                return err;
//    else                qDebug()<<device_id;

    if(usb_closePrinter)
        usb_closePrinter();
    else
        return -2;
    qDebug()<<"usb_closePrinter";
    return err;
}

static int networkWriteThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = -1;
    if(net_connetToNetDevice)
        err = net_connetToNetDevice((char*)device_uri);
    else
        return -2;
    qDebug()<<"net_connetToNetDevice:return"<<err;
//    if(err)        return err;

    if(net_wirteToNetDevice)
        err = net_wirteToNetDevice((char*)wrBuffer ,wrSize);
    else
        return -2;
    qDebug()<<"net_wirteToNetDevice:return"<<err;
//    if(wrSize != err)
//    {
//        err = -1;
//        goto ERR;
//    }else
//        err = 0;

    if(net_readFromNetDevice)
        err = net_readFromNetDevice(rdBuffer ,rdSize);
    else
        return -2;
    qDebug()<<"net_readFromNetDevice:return"<<err;
//    if(rdSize != err)
//    {
//        err = -1;
//        goto ERR;
//    }else
//        err = 0;
ERR:
    if(net_closeSocket)
        net_closeSocket();
    else
        return -2;
    qDebug()<<"net_closeSocket:";
    return err;
}

static int net_getDeviceID(const char* device_uri)
{
    char device_id[256];
    int err = -1;
    if(net_connetToNetDevice)
        err = net_connetToNetDevice((char*)device_uri);
    else
        return -2;
//    if(err)        return err;
    qDebug()<<"net_connetToNetDevice:return"<<err;

    if(net_get_device_id)
        err = net_get_device_id(device_id ,256);
    else
        return -2;
    qDebug()<<"net_get_device_id:return"<<err;
//    if(err)        return err;

    if(net_closeSocket)
        net_closeSocket();
    else
        return -2;
    qDebug()<<"net_closeSocket:";
    return err;
}

#endif


VopDevice::VopDevice(DeviceManager* dm)
    :hLLD_usb(NULL),
      hLLD_net(NULL)
{
    deviceManager = dm;
    hLLD_usb = dlopen("./libvopusb.so", RTLD_LAZY);
    if(hLLD_usb)    {
        usb_openPrinter = (int (*)(char*))dlsym(hLLD_usb, "openPrinter");
        usb_closePrinter = (void (*)(void))dlsym(hLLD_usb, "closePrinter");
        usb_USBWrite = (int (*)(char *, size_t ))dlsym(hLLD_usb, "USBWrite");
        usb_USBRead = (int (*)(char *, size_t ))dlsym(hLLD_usb, "USBRead");
        usb_get_device_id = (int (*)(char *, size_t ))dlsym(hLLD_usb, "get_device_id");
    }

    hLLD_net = dlopen("./libsocket.so", RTLD_LAZY);
    if(hLLD_net)    {
        net_connetToNetDevice = (int (*)(char*))dlsym(hLLD_net, "connetToNetDevice");
        net_closeSocket = (void (*)(void))dlsym(hLLD_net, "closeSocket");
        net_wirteToNetDevice = (int (*)(void *, int ))dlsym(hLLD_net, "wirteToNetDevice");
        net_readFromNetDevice = (int (*)(void *, int ))dlsym(hLLD_net, "readFromNetDevice");
        net_get_device_id = (int (*)(char *, size_t ))dlsym(hLLD_net, "get_device_id");
    }
}

VopDevice::~VopDevice()
{
    if(hLLD_usb)    {
        dlclose(hLLD_usb);
    }
    if(hLLD_net)    {
        dlclose(hLLD_net);
    }
}

int VopDevice::isValidDevice(const char* printer_info)
{
//    return true;
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

int VopDevice::writeThenRead(const char* device_uri ,const char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = -1;
    QString _device_uri = QString(device_uri);
    qDebug()<<__func__<<"device-uri"<<_device_uri;

    return -1;

    if(_device_uri.startsWith("usb://"))    {
        err = usbWriteThenRead(device_uri ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    }else{
        err = networkWriteThenRead(device_uri ,wrBuffer ,wrSize ,rdBuffer ,rdSize);
    }
    return err;
}

int VopDevice::getDeviceStatus(const char* device_uri)
{
    int err = -1;
    QString _device_uri = QString(device_uri);
    qDebug()<<__func__<<"device-uri"<<_device_uri;

//    return 0;
    return -1;

    if(_device_uri.startsWith("usb://"))    {
        err = usb_getDeviceID(device_uri);
    }else{
        err = net_getDeviceID(device_uri);
    }
    return err;
}

