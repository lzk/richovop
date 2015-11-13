#include "netdevice.h"
#include "log.h"
#include "vop_protocol.h"

#include <QApplication>
extern "C"{
#include "dlfcn.h"
}
static int (*hLLD_openPrinter)(char*) = NULL;
static void (*hLLD_closePrinter)(void) = NULL;
static int (*hLLD_write)(char *buffer, size_t bufsize) = NULL;
static int (*hLLD_read)(char *buffer, size_t bufsize) = NULL;
static int (*hLLD_get_device_id)(char *buffer, size_t bufsize) = NULL;

NetDevice::NetDevice()
{
    ifdelay = 0;
    QString path = QApplication::applicationDirPath();
    hLLD = dlopen((path + "/libsocket.so").toLatin1() , RTLD_LAZY);
    if(hLLD)    {
        hLLD_openPrinter  = (int (*)(char*))dlsym(hLLD, "connetToNetDevice");
        hLLD_closePrinter  = (void (*)(void))dlsym(hLLD, "closeSocket");
        hLLD_write  = (int (*)(char *, size_t ))dlsym(hLLD, "wirteToNetDevice");
        hLLD_read  = (int (*)(char *, size_t ))dlsym(hLLD, "readFromNetDevice");
        hLLD_get_device_id  = (int (*)(char *, size_t ))dlsym(hLLD, "get_device_id_net");
    }else{
        qLog("can not open net backend.");
        qLog(QString("dlerror:") + dlerror());
    }
}

NetDevice::~NetDevice()
{
}

void NetDevice::init(char*)
{

}

int NetDevice::openPrinter(const char* device_uri)
{
    if(!hLLD_openPrinter)
        return ERR_library;
    int err = hLLD_openPrinter((char*)device_uri);
    if(1 != err){
        qLog("can not open net printer");
        err = ERR_communication;
    }else
        err = ERR_ACK;
    return err;
}

int NetDevice::openPrinter(void)
{

    return ERR_ACK;
}

int NetDevice::closePrinter(void)
{
    if(!hLLD_closePrinter)
        return ERR_library;
    hLLD_closePrinter();
    return ERR_ACK;
}

int NetDevice::write(char *buffer, int bufsize)
{
    if(!hLLD_write)
        return ERR_library;
    return hLLD_write(buffer ,bufsize);
}

int NetDevice::read(char *buffer, int bufsize)
{
    if(!hLLD_read)
        return ERR_library;
    return hLLD_read(buffer ,bufsize);
}

int NetDevice::get_device_id(char *buffer, int bufsize)
{
    if(!hLLD_get_device_id)
        return ERR_library;
    int err = hLLD_get_device_id(buffer ,bufsize);
    if(1 != err){
        qLog("can not get device id");
        err = ERR_communication;
    }else
        err = ERR_ACK;
    return err;
}

int NetDevice::get_driver()
{

    return ERR_ACK;
}

int NetDevice::attach_driver()
{

    return ERR_ACK;
}

int NetDevice::write_no_read(char* wrBuffer ,int wrSize)
{
    int err = ERR_communication;
    int _write_size = 0;
    int i;
    char writeBuffer[wrSize];
    memcpy(writeBuffer ,wrBuffer ,wrSize);

    for(i = 0 ;i < 3 ;i++){
        _write_size = write(writeBuffer ,wrSize);
        qLog(QString().sprintf("write size:%d......%d" ,wrSize ,_write_size));
        if(_write_size == wrSize){
            err = ERR_ACK;
            break;
        }
    }
    return err;
}
