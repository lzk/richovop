#include "device.h"
#include "log.h"
#include "vop_protocol.h"
extern "C"{
#include "dlfcn.h"
}

Device::Device()
    :ifdelay(1)
{

}
Device::~Device()
{
    if(hLLD){
        dlclose(hLLD);
        hLLD = 0;
    }
}

int Device::write_no_read(char* wrBuffer ,int wrSize)
{
    int err = ERR_communication;
    int _write_size = 0,_read_size = 0;
    int i;
    char readBuffer[0x3ff];
    char writeBuffer[wrSize];
    memcpy(writeBuffer ,wrBuffer ,wrSize);

    for(i = 0 ;i < 3 ;i++){

        //porting from windows
        char buffer[2048];
        err = get_device_id(buffer ,sizeof(buffer));
        if(1 != err){
            char inBuffer[522];
            char outBuffer[12];
            memset(inBuffer ,0 ,sizeof(inBuffer));
            inBuffer[0] = 0x1b;
            inBuffer[1] = 0x4d;
            inBuffer[2] = 0x53;
            inBuffer[3] = 0x55;
            inBuffer[4] = 0xe0;
            inBuffer[5] = 0x2b;

            write(inBuffer ,10);
            write(&inBuffer[10] ,512);
            read(outBuffer ,sizeof(outBuffer));
        }

        _read_size = read(readBuffer ,0x3ff);
        qLog(QString().sprintf("before write clear read buffer :%d" ,_read_size));

        _write_size = write(writeBuffer ,wrSize);
        qLog(QString().sprintf("write size:%d......%d" ,wrSize ,_write_size));
        if(_write_size == wrSize){
            err = ERR_ACK;
            break;
        }else{
//            closePrinter();
        }
    }
    return err;
}

#include <unistd.h>
#define delay100ms(x) {usleep((x) * ifdelay * 100 * 1000);}
int Device ::write_then_read(char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize)
{
    int err = write_no_read(wrBuffer ,wrSize);
    delay100ms (10);

    int j;
    int _read_size = 0;
    if(!err){
        int nocheck=0;
        for(j = 0 ;j < 40 ;j++){
            if(!nocheck){
                if(1 == read(rdBuffer,1)){
                    if(0x4d != rdBuffer[0]){
                        qLog(QString().sprintf("waiting for 0x4d:%#.2x" ,rdBuffer[0]));
                        delay100ms (1);
                        continue;
                    }
                }else{
                    qLog("cannot read now,wait 100 ms read again");
                    delay100ms (1);
                    continue;
                }
            }
            nocheck = 0;
            delay100ms (1);
            if(1 == read(rdBuffer+1,1)){
                if(0x3c == rdBuffer[1]){
                    delay100ms (1);
                    _read_size = read(rdBuffer+2 ,rdSize-2);
                    j++;
                    break;
                }else if(0x4d == rdBuffer[1]){
                    nocheck = 1;
                }
            }
        }
        qLog(QString().sprintf("try times:%d" ,j));
        if(_read_size == rdSize -2){
            err = ERR_ACK;
            qLog("read complete");
        }else{
            qLog("read wrong");
        }
    }
//    closePrinter();
    return err;
}

int Device::getDeviceModel(const char* printer_info)
{
    int device = Device_invalid;
    QString str(printer_info);
    if(str.startsWith("RICOH SP 150SU v")){
        device = Device_3in1;
    }else if(str.startsWith("RICOH SP 150SUw v")){
        device = Device_3in1_wifi;
    }else if(str.startsWith("RICOH SP 150 v")){
        device = Device_sfp;
    }else if(str.startsWith("RICOH SP 150w v")){
        device = Device_sfp_wifi;
    }else  if(!str.compare("RICOH SP 150SU")){
        device = Device_3in1;
    }else if(!str.compare("RICOH SP 150SUw")){
        device = Device_3in1_wifi;
    }else if(!str.compare("RICOH SP 150")){
        device = Device_sfp;
    }else if(!str.compare("RICOH SP 150w")){
        device = Device_sfp_wifi;
    }
/*    else if(str.startsWith("Lenovo M7208W v")){
        device = Device_3in1_wifi;
    }else if(str.startsWith("Lenovo M7208 v")){
        device = Device_3in1;
    }else if(str.startsWith("Lenovo LJ2208W v")){
        device = Device_sfp_wifi;
    }else if(str.startsWith("Lenovo LJ2208 v")){
        device = Device_sfp;
    }else if(!str.compare("Lenovo M7208W"))    {
        device = Device_3in1_wifi;
    }else if(!str.compare("Lenovo M7208")){
        device = Device_3in1;
    }else if(!str.compare("Lenovo LJ2208W")){
        device = Device_sfp_wifi;
    }else if(!str.compare("Lenovo LJ2208")){
        device = Device_sfp;
    }//*/
    return device;
}
