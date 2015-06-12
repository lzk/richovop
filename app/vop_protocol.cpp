/////////////////////////////////////////
/// File:vop_protocol.cpp
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include <string.h>
#include "vop_protocol.h"
#include "devicemanager.h"

#pragma pack(1)
typedef struct _COMM_HEADER
{
    UINT32 magic;
    UINT16 id;
    UINT16 len;     // lenght of data structure and its data

    // data structure
    UINT8 subid;
    UINT8 len2;

    // data append
    UINT8 subcmd;
}COMM_HEADER;

#define vop_copy(buffer)                            vop_cmd(_LS_CPYCMD ,1 ,buffer ,sizeof(*buffer))
#define vop_getWifiInfo(buffer)                 vop_cmd(_LS_WIFICMD ,0 ,buffer ,sizeof(*buffer))
#define vop_setWifiInfo(buffer)                 vop_cmd(_LS_WIFICMD ,1 ,buffer ,sizeof(*buffer))
#define vop_getApList(buffer)                    vop_cmd(_LS_WIFICMD ,0x07 ,buffer ,sizeof(*buffer))
#define vop_setPasswd(buffer)                  vop_cmd(_LS_PRNCMD ,0x06 ,buffer ,sizeof(*buffer))
#define vop_getPasswd(buffer)                  vop_cmd(_LS_PRNCMD ,0x07 ,buffer ,sizeof(*buffer))
#define vop_confirmPasswd(buffer)          vop_cmd(_LS_PRNCMD ,0x08 ,buffer ,sizeof(*buffer))

#define     MAGIC_NUM           0x1A2B3C4D
static const unsigned char INIT_VALUE = 0xfe;
///
/// \brief vop_getCmdDirect
/// \param cmd
/// \param sub_cmd
/// \return 0:get 1:set
///
static int vop_getCmdDirect(CMD_ID cmd ,int sub_cmd)
{
    int direct = 0;
    switch(cmd){
    case _LS_CPYCMD:        direct = sub_cmd;        break;
    case _LS_WIFICMD:
        switch(sub_cmd){
        case 0:             direct = 0;        break;//get
        case 1:             direct = 1;        break;//set
        case 0x07:       direct = 0;        break;//get aplist
        }
        break;
    case _LS_PRNCMD:
        switch(sub_cmd){
        case 0x06:  direct = 1; break;//set passwd
        case 0x07:  direct = 0; break;//get passwd
        case 0x08:  direct = 1; break;//comfirm
        }
        break;

    case _LS_SEARCH:
    case _LS_CONNECT	:
    case _LS_DISCONNECT:
    case _LS_CALLFUNC:
    case _LS_SHOWVAR:
    case _LS_MODIFYVAR:
    case _LS_PRIVEXEC:
    case _LS_ENGCMD:
    case _LS_NETCMD:
    case _LS_SCACMD:
    case _LS_FAXCMD:
    case _LS_DBGMSG:
    case _LS_HEARTBEAT:
    case _LS_PANKEY:
    case _LS_PANIMG:
    case _LS_DATADOWN:
    case _LS_DATAUPLD:
    default:
        break;
    }
    return direct;
}

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

VopProtocol::VopProtocol(DeviceManager* dm)
    : copy_parameter(new copycmdset),
      wifi_parameter(new cmdst_wifi_get),
      wifi_aplist(new cmdst_aplist_get),
      passwd(new cmdst_passwd)
{
    deviceManager = dm;
    copy_set_defaultPara();
    memset(wifi_parameter ,0 ,sizeof(cmdst_wifi_get));
    memset(wifi_aplist ,0 ,sizeof(*wifi_aplist));
    memset(passwd ,0 ,sizeof(cmdst_passwd));
    //for test
    strcpy(wifi_aplist->aplist[0].ssid ,"123");
    wifi_aplist->aplist[0].encryption = 0;
    strcpy(wifi_aplist->aplist[1].ssid ,"456");
    wifi_aplist->aplist[1].encryption = 1;
    strcpy(wifi_aplist->aplist[2].ssid ,"789");
    wifi_aplist->aplist[2].encryption = 2;
    strcpy(wifi_aplist->aplist[3].ssid ,"123456");
    wifi_aplist->aplist[3].encryption = 3;
    strcpy(wifi_parameter->ssid ,"789");
}

VopProtocol::~VopProtocol()
{
    delete copy_parameter;
    delete wifi_parameter;
    delete wifi_aplist;
    delete passwd;
}

const char* VopProtocol::getErrString(int err)
{
    const char* str = NULL;
    switch(err)    {
    case ERR_ACK :
        str = QT_TR_NOOP_UTF8("ACK");
        break;
    case ERR_CMD_invalid :
        str = QT_TR_NOOP_UTF8("CMD invalid");
        break;
    case ERR_Parameter_invalid :
        str = QT_TR_NOOP_UTF8("Parameter invalid");
        break;
    case ERR_Printer_busy :
        str = QT_TR_NOOP_UTF8("Printer busy");
        break;
    case ERR_Printer_error :
        str = QT_TR_NOOP_UTF8("Printer error");
        break;
    case ERR_Set_parameter_error :
        str = QT_TR_NOOP_UTF8("Set parameter error");
        break;
    case ERR_Get_parameter_error :
        str = QT_TR_NOOP_UTF8("Get parameter error");
        break;
    case ERR_Printer_is_Sleeping:
        str = QT_TR_NOOP_UTF8("Printer is Sleeping");
        break;
    case ERR_Printer_is_in_error:
        str = QT_TR_NOOP_UTF8("Printer is in error");
        break;
    case ERR_Password_incorrect :
        str = QT_TR_NOOP_UTF8("Password incorrect");
        break;
    case ERR_Scanner_operation_NG :
        str = QT_TR_NOOP_UTF8("Scanner operation NG");
        break;
    case ERR_communication :
        str = QT_TR_NOOP_UTF8("communication error");
        break;
    case ERR_Do_not_support :
    default:
        str = QT_TR_NOOP_UTF8("Do not support");
        break;
    }
    return str;
}

int VopProtocol::vop_cmd(CMD_ID cmd ,int sub_cmd, void* data ,int data_size)
{
    int result = 0;

    int direct = vop_getCmdDirect(cmd ,sub_cmd);

    int device_cmd_len = sizeof(COMM_HEADER)+data_size;
    char* buffer = new char[device_cmd_len];
    memset( buffer, INIT_VALUE, sizeof(device_cmd_len));
    COMM_HEADER* ppkg = reinterpret_cast<COMM_HEADER*>( buffer );

    ppkg->magic = MAGIC_NUM ;
    ppkg->id = cmd;
    ppkg->len = 3+data_size * direct;

    // For the simple data setting, e.g. copy/scan/prn/wifi/net, SubID is always 0x13, len is always 0x01,
    // it just stand for the sub id. The real data length is defined by the lib
    ppkg->subid = 0x13;
    ppkg->len2 = 1;
    ppkg->subcmd = sub_cmd;
    memcpy(buffer + sizeof(COMM_HEADER) ,data ,data_size);
//    result = deviceManager->device_writeThenRead(buffer ,sizeof(COMM_HEADER)+data_size * direct
//                                               ,buffer ,sizeof(COMM_HEADER)+data_size * direct);
    result = DeviceManager::device_writeThenRead(buffer ,sizeof(COMM_HEADER)+data_size * direct
                                               ,buffer ,sizeof(COMM_HEADER)+data_size * direct);
    //check rusult
    if(!result && MAGIC_NUM == ppkg->magic){
        result = ppkg->subcmd;
        if(!direct){//read
            deviceManager->mutex.lock();
            memcpy(data ,buffer + sizeof(COMM_HEADER) ,data_size);
            deviceManager->mutex.unlock();
        }
    }
    return result;
}

void VopProtocol::copy_set_defaultPara()
{    
    QMutexLocker locker(&deviceManager->mutex);
    memcpy(copy_parameter ,&default_copy_parameter ,sizeof(default_copy_parameter));
}

void VopProtocol::copy_set_para(copycmdset* p)
{
    QMutexLocker locker(&deviceManager->mutex);
    memcpy(copy_parameter ,p ,sizeof(copycmdset));
}

copycmdset VopProtocol::copy_get_para()
{
    QMutexLocker locker(&deviceManager->mutex);
    return *copy_parameter;
}

int  VopProtocol::cmd_copy()
{
    deviceManager->mutex.lock();
    copycmdset _para = *copy_parameter;
    deviceManager->mutex.unlock();
    return vop_copy(&_para);
}

void VopProtocol::wifi_set_ssid(cmdst_wifi_get* p ,const char* ssid)
{
    memset(p->ssid ,0 ,32);
    strcpy(p->ssid ,ssid);
}

void VopProtocol::wifi_set_password(cmdst_wifi_get* p ,const char* password)
{
    memset(p->pwd ,0 ,64);
    strcpy(p->pwd ,password);
}

void VopProtocol::wifi_set_para(cmdst_wifi_get* p)
{
    QMutexLocker locker(&deviceManager->mutex);
    memcpy(wifi_parameter ,p ,sizeof(cmdst_wifi_get));
}

cmdst_wifi_get VopProtocol::wifi_get_para()
{
    QMutexLocker locker(&deviceManager->mutex);
    return *wifi_parameter;
}
cmdst_aplist_get VopProtocol::wifi_getAplist()
{
    QMutexLocker locker(&deviceManager->mutex);
    return *wifi_aplist;
}

int  VopProtocol::cmd_wifi_get()
{
    deviceManager->mutex.lock();
    cmdst_wifi_get _para = *wifi_parameter;
    deviceManager->mutex.unlock();
    return vop_getWifiInfo(&_para);
}

int VopProtocol::cmd_wifi_set()
{
    deviceManager->mutex.lock();
    cmdst_wifi_get _para = *wifi_parameter;
    deviceManager->mutex.unlock();
    return vop_setWifiInfo(&_para);
}

int  VopProtocol::cmd_wifi_getAplist()
{
    deviceManager->mutex.lock();
    cmdst_aplist_get _para = *wifi_aplist;
    deviceManager->mutex.unlock();
    return vop_getApList(&_para);
}

void VopProtocol::passwd_set(const char* p)
{
    memset(passwd ,0 ,32);
    strcpy(passwd->passwd ,p);
}

int VopProtocol::cmd_passwd_set()
{
    deviceManager->mutex.lock();
    cmdst_passwd _para = *passwd;
    deviceManager->mutex.unlock();
    return vop_setPasswd(&_para);
}

int VopProtocol::cmd_passwd_get()
{
    deviceManager->mutex.lock();
    cmdst_passwd _para = *passwd;
    deviceManager->mutex.unlock();
    return vop_getPasswd(&_para);
}

int VopProtocol::cmd_passwd_confirm()
{
    deviceManager->mutex.lock();
    cmdst_passwd _para = *passwd;
    deviceManager->mutex.unlock();
    return vop_confirmPasswd(&_para);
}
