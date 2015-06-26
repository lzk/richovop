/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include <string.h>
#include "vop_protocol.h"
#include "devicemanager.h"
#include "devicecontrol.h"
#include "app/log.h"

static int _base64_char_value(char base64char)
 {
    if (base64char >= 'A' && base64char <= 'Z')
         return base64char-'A';
    if (base64char >= 'a' && base64char <= 'z')
         return base64char-'a'+26;
    if (base64char >= '0' && base64char <= '9')
         return base64char-'0'+2*26;
    if (base64char == '+')
         return 2*26+10;
    if (base64char == '/')
         return 2*26+11;
    return -1;
}

static int _base64_decode_triple(char quadruple[4], unsigned char *result)
 {
    int i, triple_value, bytes_to_decode = 3, only_equals_yet = 1;
    int char_value[4];

    for (i=0; i<4; i++)
         char_value[i] = _base64_char_value(quadruple[i]);

    for (i=3; i>=0; i--)
    {
         if (char_value[i]<0)
         {
             if (only_equals_yet && quadruple[i]=='=')
             {
                  char_value[i]=0;
                  bytes_to_decode--;
                  continue;
             }
             return 0;
         }
         only_equals_yet = 0;
    }

    if (bytes_to_decode < 0)
         bytes_to_decode = 0;

    triple_value = char_value[0];
    triple_value *= 64;
    triple_value += char_value[1];
    triple_value *= 64;
    triple_value += char_value[2];
    triple_value *= 64;
    triple_value += char_value[3];

    for (i=bytes_to_decode; i<3; i++)
         triple_value /= 256;
    for (i=bytes_to_decode-1; i>=0; i--)
    {
         result[i] = triple_value%256;
         triple_value /= 256;
    }

    return bytes_to_decode;
}

static size_t Base64Decode(char *source, unsigned char *target, size_t targetlen)
 {
    char *src, *tmpptr;
    char quadruple[4], tmpresult[3];
    int i; size_t tmplen = 3;
    size_t converted = 0;

    src = (char *)malloc(strlen(source)+5);
    if (src == NULL)
         return -1;
    strcpy(src, source);
    strcat(src, "====");
    tmpptr = src;
    while (tmplen == 3)
    {
         /* get 4 characters to convert */
         for (i=0; i<4; i++)
         {
             while (*tmpptr != '=' && _base64_char_value(*tmpptr)<0)
                  tmpptr++;
             quadruple[i] = *(tmpptr++);
         }
         tmplen = _base64_decode_triple(quadruple, (unsigned char*)tmpresult);
         if (targetlen < tmplen)
         {
             free(src);
             return converted;
         }
         memcpy(target, tmpresult, tmplen);
         target += tmplen;
         targetlen -= tmplen;
         converted += tmplen;
    }
    free(src);
    return converted;
}

int VopProtocol::DecodeStatusFromDeviceID(char* device_id, PRINTER_STATUS* status)
{
    if (device_id==NULL || status==NULL) {
        return -1;
    }
    char *p = device_id;

    while (*p && strncmp(p,"STS:",4)!=0) // Look for "STS:"
        p++;

    if (!*p)	{ // "STS:" not found
        qLog()<<"STS: not found";
        return -1;
    }
    p += 4;	// Skip "STS:"

    if (Base64Decode(p, (unsigned char*)status, sizeof(PRINTER_STATUS)) != sizeof(PRINTER_STATUS))
    {
        return -1;
    }
    return 0;
}

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

typedef enum _CMD_ID
{
    _LS_SEARCH		= 0x0000, 	// Search Machine
    _LS_CONNECT		= 0x0001,	// Connect Machine
    _LS_DISCONNECT	= 0x0002,	// Disconnect Machine
    _LS_CALLFUNC	= 0x0100,	// Call Function
    _LS_SHOWVAR		= 0x0101, 	// Show Variable/Structure
    _LS_MODIFYVAR	= 0x0102,	// Modify Variable/Structure
    _LS_PRIVEXEC	= 0x0103,	// Private Execution
    _LS_ENGCMD		= 0x0104,	// Engine Command
    _LS_NETCMD		= 0x0105,	// Network Command
    _LS_WIFICMD		= 0x0106,	// Wireless Command
    _LS_PRNCMD		= 0x0107,	// Print Command
    _LS_SCACMD		= 0x0108,	// Scan Command
    _LS_CPYCMD		= 0x0109,	// Copy Command
    _LS_FAXCMD		= 0x010A,	// Fax Command
    _LS_DBGMSG		= 0x0200, 	// Debug Message
    _LS_HEARTBEAT 	= 0x0201,	// Heart Beat, Null Packet, keep activated
    _LS_PANKEY		= 0x0300,	// Panel Key Simulation
    _LS_PANIMG		= 0x0301,	// Panel Frame & LED status
    _LS_DATADOWN	= 0x0400,	// Download Data
    _LS_DATAUPLD	= 0x0401	// Upload Data
}CMD_ID;
#define vop_copy(buffer)                            vop_cmd(_LS_CPYCMD ,1 ,buffer ,sizeof(*buffer))
#define vop_getWifiInfo(buffer)                 vop_cmd(_LS_WIFICMD ,0 ,buffer ,sizeof(*buffer))
#define vop_setWifiInfo(buffer)                 vop_cmd(_LS_WIFICMD ,1 ,buffer ,sizeof(*buffer))
#define vop_getApList(buffer)                    vop_cmd(_LS_WIFICMD ,0x07 ,buffer ,sizeof(*buffer))
#define vop_setPasswd(buffer)                  vop_cmd(_LS_PRNCMD ,0x06 ,buffer ,sizeof(*buffer))
#define vop_getPasswd(buffer)                  vop_cmd(_LS_PRNCMD ,0x07 ,buffer ,sizeof(*buffer))
#define vop_confirmPasswd(buffer)          vop_cmd(_LS_PRNCMD ,0x08 ,buffer ,sizeof(*buffer))
///
/// \brief vop_getCmdDirect
/// \param cmd
/// \param sub_cmd
/// \return 0:get 1:set
///
static int vop_getCmdDirect(int cmd ,int sub_cmd ,int& direct ,int& data_buffer_size)
{
    int ret = 0;
    switch(cmd){
    case _LS_CPYCMD:        direct = sub_cmd;data_buffer_size = 128;        break;
    case _LS_WIFICMD:
        switch(sub_cmd){
        case 0:             direct = 0;data_buffer_size = 180;        break;//get
        case 1:             direct = 1;data_buffer_size = 180;        break;//set
        case 0x07:       direct = 0;data_buffer_size = 340;        break;//get aplist
        }
        break;
    case _LS_PRNCMD:
        switch(sub_cmd){
        case 0x06:  direct = 1;data_buffer_size = 32;  break;//set passwd
        case 0x07:  direct = 0;data_buffer_size = 32; break;//get passwd
        case 0x08:  direct = 1;data_buffer_size = 32; break;//comfirm
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
        ret = -1;
        break;
    }
    return ret;
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
    : status(new PRINTER_STATUS),
      copy_parameter(new copycmdset),
      wifi_parameter(new cmdst_wifi_get),
      wifi_aplist(new cmdst_aplist_get),
      passwd(new cmdst_passwd),
      device_manager(dm)
{
    memcpy(copy_parameter ,&default_copy_parameter ,sizeof(default_copy_parameter));
    memset(wifi_parameter ,0 ,sizeof(cmdst_wifi_get));
    memset(wifi_aplist ,0 ,sizeof(*wifi_aplist));
    memset(passwd ,0 ,sizeof(cmdst_passwd));
    //for test
//    strcpy(wifi_aplist->aplist[0].ssid ,"123");
//    wifi_aplist->aplist[0].encryption = 0;
//    strcpy(wifi_aplist->aplist[1].ssid ,"456");
//    wifi_aplist->aplist[1].encryption = 1;
//    strcpy(wifi_aplist->aplist[2].ssid ,"789");
//    wifi_aplist->aplist[2].encryption = 2;
//    strcpy(wifi_aplist->aplist[3].ssid ,"123456");
//    wifi_aplist->aplist[3].encryption = 3;
//    strcpy(wifi_parameter->ssid ,"789");
}

VopProtocol::~VopProtocol()
{
    delete status;
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
    case ERR_library :
        str = QT_TR_NOOP_UTF8("library error");
        break;
    case ERR_Do_not_support :
    default:
        str = QT_TR_NOOP_UTF8("Do not support");
        break;
    }
    return str;
}

PRINTER_STATUS VopProtocol::get_status()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *status;
}
int VopProtocol::get_deviceStatus()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return status->PrinterStatus;
}


#define     MAGIC_NUM           0x1A2B3C4D
#define change_32bit_edian(x) (((x) << 24 & 0xff000000) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | (((x) >> 24) & 0xff))
#define change_16bit_edian(x) (((x) << 8) & 0xff00 | ((x) >> 8) & 0x00ff)
static const unsigned char INIT_VALUE = 0xfe;

int VopProtocol::vop_cmd(int cmd ,int sub_cmd, void* data ,int data_size)
{
    int direct=0 ,data_buffer_size=0;
    int err = vop_getCmdDirect(cmd ,sub_cmd ,direct ,data_buffer_size);
    if(err)
        return -3;//not support

    int device_cmd_len = sizeof(COMM_HEADER)+data_buffer_size;
    char* buffer = new char[device_cmd_len];
    memset( buffer, INIT_VALUE, sizeof(device_cmd_len));
    COMM_HEADER* ppkg = reinterpret_cast<COMM_HEADER*>( buffer );

    ppkg->magic = MAGIC_NUM ;
    ppkg->id = cmd;
    ppkg->len = 3+data_buffer_size * direct;

    // For the simple data setting, e.g. copy/scan/prn/wifi/net, SubID is always 0x13, len is always 0x01,
    // it just stand for the sub id. The real data length is defined by the lib
    ppkg->subid = 0x13;
    ppkg->len2 = 1;
    ppkg->subcmd = sub_cmd;
    device_manager->mutex_ctrl.lock();
    memcpy(buffer + sizeof(COMM_HEADER) ,data ,data_size);
    device_manager->mutex_ctrl.unlock();

//    qLog("Write:%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x"
//           ,buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],
//            buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10]);
    err = DeviceContrl::device_writeThenRead(buffer ,sizeof(COMM_HEADER)+data_buffer_size * direct
                                               ,buffer ,sizeof(COMM_HEADER)+data_buffer_size * (1 - direct));
//    qLog("read:%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x-%#.2x"
//           ,buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],
//            buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10]);
    //check result
    if(!err && MAGIC_NUM == ppkg->magic){//ACK
        if(direct){//set
                err = ppkg->subcmd;
        }else{//get
            device_manager->mutex_ctrl.lock();
            memcpy(data ,buffer + sizeof(COMM_HEADER) ,data_size);
            device_manager->mutex_ctrl.unlock();
        }
    }else
        err = -1;
    delete buffer;
    return err;
}

void VopProtocol::copy_set_defaultPara()
{    
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(copy_parameter ,&default_copy_parameter ,sizeof(default_copy_parameter));
}

void VopProtocol::copy_set_para(copycmdset* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(copy_parameter ,p ,sizeof(copycmdset));
}

copycmdset VopProtocol::copy_get_para()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *copy_parameter;
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
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(wifi_parameter ,p ,sizeof(cmdst_wifi_get));
}

cmdst_wifi_get VopProtocol::wifi_get_para()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *wifi_parameter;
}
cmdst_aplist_get VopProtocol::wifi_getAplist()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *wifi_aplist;
}

void VopProtocol::passwd_set(const char* p)
{
    memset(passwd ,0 ,32);
    strcpy(passwd->passwd ,p);
}

int VopProtocol::cmd(int _cmd)
{
    int err = -3;

    switch(_cmd)    {
    case CMD_GetStatus:{
        char buffer[256];
        err = DeviceContrl::device_getDeviceStatus(buffer ,sizeof(buffer));
        if(!err){
            DecodeStatusFromDeviceID(buffer ,status);
        }
        break;
    }
    case CMD_COPY:{
        err = vop_copy(copy_parameter);
    }
        break;
    case CMD_WIFI_apply:{
        err =  vop_setWifiInfo(wifi_parameter);
    }
        break;
    case CMD_WIFI_get:{
        err = vop_getWifiInfo(wifi_parameter);
    }
        break;
    case CMD_WIFI_getAplist:{
        err = vop_getApList(wifi_aplist);
    }
        break;
    case CMD_PASSWD_set:{
        err = vop_setPasswd(passwd);
    }
        break;
    case CMD_PASSWD_get:{
        err = vop_getPasswd(passwd);
    }
        break;
    case CMD_PASSWD_confirm:{
        err = vop_confirmPasswd(passwd);
    }
        break;
    default:
        break;
    }
    return err;
}
# if 0
int  VopProtocol::cmd_copy()
{
    device_manager->mutex_ctrl.lock();
    copycmdset _para = *copy_parameter;
    device_manager->mutex_ctrl.unlock();

    _para.scale = change_16bit_edian(_para.scale);
    return vop_copy(&_para);
}

int  VopProtocol::cmd_wifi_get()
{
    device_manager->mutex_ctrl.lock();
    cmdst_wifi_get _para = *wifi_parameter;
    device_manager->mutex_ctrl.unlock();
    return vop_getWifiInfo(&_para);
}

int VopProtocol::cmd_wifi_set()
{
    device_manager->mutex_ctrl.lock();
    cmdst_wifi_get _para = *wifi_parameter;
    device_manager->mutex_ctrl.unlock();
    return vop_setWifiInfo(&_para);
}

int  VopProtocol::cmd_wifi_getAplist()
{
    device_manager->mutex_ctrl.lock();
    cmdst_aplist_get _para = *wifi_aplist;
    device_manager->mutex_ctrl.unlock();
    return vop_getApList(&_para);
}

int VopProtocol::cmd_passwd_set()
{
    device_manager->mutex_ctrl.lock();
    cmdst_passwd _para = *passwd;
    device_manager->mutex_ctrl.unlock();
    return vop_setPasswd(&_para);
}

int VopProtocol::cmd_passwd_get()
{
    device_manager->mutex_ctrl.lock();
    cmdst_passwd _para = *passwd;
    device_manager->mutex_ctrl.unlock();
    return vop_getPasswd(&_para);
}

int VopProtocol::cmd_passwd_confirm()
{
    device_manager->mutex_ctrl.lock();
    cmdst_passwd _para = *passwd;
    device_manager->mutex_ctrl.unlock();
    return vop_confirmPasswd(&_para);
}
#endif
