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

int VopProtocol::getDESfromDeviceID(char* device_id ,char* str)
{
    if (device_id==NULL) {
        return -1;
    }
    char *p = device_id;

    while (*p && strncmp(p,"DES:",4)!=0) // Look for "DES:"
        p++;

    if (!*p)	{ // "DES:" not found
        _Q_LOG("DES: not found");
        return -1;
    }
    p += 4;	// Skip "DES:"

    char* q = p;
    while(*q && ';'!= *q)
        q++;
    if(!*q){
        _Q_LOG("DES:no \';\'");
        return -1;
    }

    memcpy(str ,p ,q-p);
    str[q - p] = 0;
    return 0;
}

int VopProtocol::getStatusFromDeviceID(char* device_id)
{
    int err = ERR_ACK;
    if(DecodeStatusFromDeviceID(device_id ,status))
        err = ERR_decode_status;
    else{
        PRINTER_STATUS sts = get_status();
        C_LOG("get_deviceStatus correct:%#.2x" ,sts.PrinterStatus);
        if(sts.TonelStatusLevelK & 0x80){
            _Q_LOG("Tonel not installed");
        }else{
            C_LOG("Tonel has been installed, status:%d" ,sts.TonelStatusLevelK);
        }
        switch(sts.PaperTrayStatus){
        case 0:            _Q_LOG("paper tray status: ready");            break;
        case 0xff:
            _Q_LOG("paper tray status: tray no installed");            break;
        case 0xfe:
            _Q_LOG("paper tray status: out of page");            break;
        default:            break;
        }
        switch(sts.job){
        case 0:            _Q_LOG("executing job: unknow job");            break;
        case 1:            _Q_LOG("executing job: print job");            break;
        case 2:            _Q_LOG("executing job: normal copy job");            break;
        case 3:            _Q_LOG("executing job: scan job");            break;
        case 4:            _Q_LOG("executing job: fax job");            break;
        case 5:            _Q_LOG("executing job: ifax job");            break;
        case 6:            _Q_LOG("executing job: report job");            break;
        case 7:            _Q_LOG("executing job: n in 1 copy job");            break;
        case 8:            _Q_LOG("executing job: id card copy job");            break;
        case 9:            _Q_LOG("executing job: id card copy mode");            break;
        default:            _Q_LOG("executing job: print job");            break;
        }

//        QString owner_name;
//        owner_name.setRawData((const QChar*)sts.OwnerName ,16);
//        _Q_LOG("owner name:" + owner_name);
//        QString docu_name;
//        docu_name.setRawData((const QChar*)sts.DocuName ,16);
//        _Q_LOG("docu name:" + docu_name);
        switch(sts.PrinterStatus){
        case PSTATUS_Ready:
            err = STATUS_ready;
            break;
        case PSTATUS_PowerSaving:
            err = STATUS_sleep;
            break;
        case PSTATUS_CopyScanNextPage:
            if(sts.job == 8)
                err = STATUS_IDCardCopyTurnCardOver;
            else
                err = STATUS_CopyScanNextPage;
            break;

        case PSTATUS_Printing:
        case PSTATUS_PrintCanceling:
            err = STATUS_busy_printing;
            break;
        case PSTATUS_CopyScanning:
        case PSTATUS_CopyPrinting:
        case PSTATUS_CopyCanceling:
        case PSTATUS_ScanScanning:
        case PSTATUS_ScanSending:
        case PSTATUS_ScanCanceling:
        case PSTATUS_ScannerBusy:
            err = STATUS_busy_scanningOrCoping;
            break;
        case PSTATUS_InitializeJam:
        case PSTATUS_NofeedJam:
        case PSTATUS_JamAtRegistStayOn:
        case PSTATUS_JamAtExitNotReach:
        case PSTATUS_JamAtExitStayOn:
            err = STATUS_jam;
            break;
        case PSTATUS_TonerEnd1:
        case PSTATUS_TonerEnd2:
        case PSTATUS_WasteTonerFull:
            err = STATUS_TonerEnd;
            break;
        default:
            err = STATUS_other;
            break;
        }
    }
    return err;
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
        _Q_LOG("STS: not found");
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
        case 0x08:       direct = 0;data_buffer_size = 1;            break;//get wifi status
        }
        break;
    case _LS_PRNCMD:
        switch(sub_cmd){
        case 0x00:  direct = 0;data_buffer_size = 1;  break;//get psave time
        case 0x01:  direct = 1;data_buffer_size = 1;  break;//set psave time
        case 0x06:  direct = 1;data_buffer_size = 32;  break;//set passwd
        case 0x07:  direct = 0;data_buffer_size = 32; break;//get passwd
        case 0x08:  direct = 1;data_buffer_size = 32; break;//comfirm
        case 0x0e:  direct = 0;data_buffer_size = 1; break;//get power off time
        case 0x0f:  direct = 1;data_buffer_size = 1; break;//set power off time
        case 0x11:  direct = 0;data_buffer_size = 1; break;//get toner end
        case 0x12:  direct = 1;data_buffer_size = 1; break;//set toner end
        }
        break;
    case _LS_NETCMD:
        switch(sub_cmd){
        case 0x00:  direct = 0;data_buffer_size = 128;  break;//get ipv4
        case 0x01:  direct = 1;data_buffer_size = 128;  break;//set ipv4
        case 0x02:  direct = 0;data_buffer_size = 340;  break;//get ipv6
        case 0x03:  direct = 1;data_buffer_size = 340;  break;//set ipv6
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
    2,//UINT8 Density         ; // 0  -   0~6
    1,//UINT8 copyNum         ; // 1  -   1~99
    0,//UINT8 scanMode        ; // 2  -   0: Photo, 1: Text, 2: ID card
    0,//UINT8 orgSize         ; // 3  -   0: A4, 1: A5, 2: B5, 3: Letter, 4: Executive
    1,//UINT8 paperSize       ; // 4  -   0: Letter, 1: A4, 2: A5, 3: A6, 4: B5, 5: B6, 6: Executive, 7: 16K
    0,//UINT8 nUp             ; // 5  -   0:1up, 1: 2up, 3: 4up, 4: 9up
    0,//UINT8 dpi             ; // 6  -   0: 300*300, 1: 600*600
    0,//UINT8 mediaType       ; // 7  -   0: plain paper 1: Recycled paper 2: Thick paper 3: Thin paper 4: Label
    100,//UINT16 scale          ; // 8  -   25~400, disabled for 2/4/9up
};

VopProtocol::VopProtocol(DeviceManager* dm)
    :
      device_manager(dm),
      status(new PRINTER_STATUS),
      copy_parameter(new copycmdset),
      wifi_parameter(new cmdst_wifi_get),
      wifi_aplist(new cmdst_aplist_get),
      passwd(new cmdst_passwd),
      wifi_status(new cmdst_wifi_status),
      tonerEnd(new cmdst_tonerEnd),
      pSaveTime(new cmdst_PSave_time),
      powerOffTime(new cmdst_powerOff_time),
      ip_info(new net_info_st),
      ipv6_info(new net_ipv6_st)
{
    memcpy(copy_parameter ,&default_copy_parameter ,sizeof(default_copy_parameter));
    memset(wifi_parameter ,0 ,sizeof(cmdst_wifi_get));
    memset(wifi_aplist ,0 ,sizeof(*wifi_aplist));
    memset(passwd ,0 ,sizeof(cmdst_passwd));
    memset(wifi_status ,0 ,sizeof(cmdst_wifi_status));
    memset(tonerEnd ,0 ,sizeof(cmdst_tonerEnd));
    memset(pSaveTime ,0 ,sizeof(cmdst_PSave_time));
    memset(powerOffTime ,0 ,sizeof(cmdst_powerOff_time));
    memset(ip_info ,0 ,sizeof(net_info_st));
    memset(ipv6_info ,0 ,sizeof(net_ipv6_st));

}

VopProtocol::~VopProtocol()
{
    delete status;
    delete copy_parameter;
    delete wifi_parameter;
    delete wifi_aplist;
    delete passwd;
    delete wifi_status;
    delete tonerEnd;
    delete pSaveTime;
    delete powerOffTime;
    delete ip_info;
    delete ipv6_info;
}

//#define STR_PREFIX  QT_TR_NOOP_UTF8
#define STR_PREFIX
const char* VopProtocol::getErrString(int err)
{
    const char* str = NULL;
    switch(err)    {
    case ERR_ACK :
        str = STR_PREFIX("ACK");
        break;
    case ERR_CMD_invalid :
        str = STR_PREFIX("CMD invalid");
        break;
    case ERR_Parameter_invalid :
        str = STR_PREFIX("Parameter invalid");
        break;
    case ERR_Printer_busy :
        str = STR_PREFIX("Printer busy");
        break;
    case ERR_Printer_error :
        str = STR_PREFIX("Printer error");
        break;
    case ERR_Set_parameter_error :
        str = STR_PREFIX("Set parameter error");
        break;
    case ERR_Get_parameter_error :
        str = STR_PREFIX("Get parameter error");
        break;
    case ERR_Printer_is_Sleeping:
        str = STR_PREFIX("Printer is Sleeping");
        break;
    case ERR_Printer_is_in_error:
        str = STR_PREFIX("Printer is in error");
        break;
    case ERR_Password_incorrect :
        str = STR_PREFIX("Password incorrect");
        break;
    case ERR_Scanner_operation_NG :
        str = STR_PREFIX("Scanner operation NG");
        break;
    case ERR_communication :
        str = STR_PREFIX("VOP defined error: communication error");
        break;
    case ERR_library :
        str = STR_PREFIX("VOP defined error: library error");
        break;
    case ERR_Do_not_support :
        str = STR_PREFIX("FW do not support");
        break;
    case ERR_decode_status:
        str = STR_PREFIX("VOP defined error: decode status err");
        break;
    case ERR_decode_device:
        str = STR_PREFIX("VOP defined error: decode device err");
        break;
    case ERR_wifi_have_not_been_inited:
        str = STR_PREFIX("VOP defined error: wifi have not been inited");
        break;
    case ERR_WIFI_SET_SSID:
        str = STR_PREFIX("VOP defined error: wifi set ssid");
        break;
    case STATUS_ready:
        str = STR_PREFIX("VOP ACK status: ready");
        break;
    case STATUS_sleep:
        str = STR_PREFIX("VOP ACK status: sleep");
        break;
    case STATUS_busy_printing:
        str = STR_PREFIX("VOP ACK status: printing");
        break;
    case STATUS_busy_scanningOrCoping:
        str = STR_PREFIX("VOP ACK status: scanning or coping");
        break;
    case STATUS_CopyScanNextPage:
        str = STR_PREFIX("VOP ACK status: copy scan next page");
        break;
    case STATUS_jam:
        str = STR_PREFIX("VOP ACK status: jam");
        break;
    case STATUS_TonerEnd:
        str = STR_PREFIX("VOP ACK status: toner end");
        break;
    case STATUS_other:
        str = STR_PREFIX("VOP ACK status: other status");
        break;
    case ERR_vop_cannot_support:
    default:
        str = STR_PREFIX("VOP defined error: VOP do not support");
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

#define vop_getCopy(buffer)                      vop_cmd(_LS_CPYCMD ,0 ,buffer ,sizeof(*buffer))
#define vop_copy(buffer)                            vop_cmd(_LS_CPYCMD ,1 ,buffer ,sizeof(*buffer))
#define vop_getWifiInfo(buffer)                 vop_cmd(_LS_WIFICMD ,0 ,buffer ,sizeof(*buffer))
#define vop_setWifiInfo(buffer)                 vop_cmd(_LS_WIFICMD ,1 ,buffer ,sizeof(*buffer))
#define vop_setWifiInfo_noread(buffer)             vop_cmd(_LS_WIFICMD ,0xff ,buffer ,sizeof(*buffer))
#define vop_getApList(buffer)                    vop_cmd(_LS_WIFICMD ,0x07 ,buffer ,sizeof(*buffer))
#define vop_getWifiStatus(buffer)             vop_cmd(_LS_WIFICMD ,0x08 ,buffer ,sizeof(*buffer))
#define vop_setPasswd(buffer)                  vop_cmd(_LS_PRNCMD ,0x06 ,buffer ,sizeof(*buffer))
#define vop_getPasswd(buffer)                  vop_cmd(_LS_PRNCMD ,0x07 ,buffer ,sizeof(*buffer))
#define vop_confirmPasswd(buffer)          vop_cmd(_LS_PRNCMD ,0x08 ,buffer ,sizeof(*buffer))
#define vop_getTonerEnd(buffer)               vop_cmd(_LS_PRNCMD ,0x11 ,buffer ,sizeof(*buffer))
#define vop_setTonerEnd(buffer)               vop_cmd(_LS_PRNCMD ,0x12 ,buffer ,sizeof(*buffer))
#define vop_getPsaveTime(buffer)             vop_cmd(_LS_PRNCMD ,0x00 ,buffer ,sizeof(*buffer))
#define vop_setPsaveTime(buffer)             vop_cmd(_LS_PRNCMD ,0x01 ,buffer ,sizeof(*buffer))
#define vop_getPowerOff(buffer)               vop_cmd(_LS_PRNCMD ,0x0e ,buffer ,sizeof(*buffer))
#define vop_setPowerOff(buffer)               vop_cmd(_LS_PRNCMD ,0x0f ,buffer ,sizeof(*buffer))
#define vop_getv4(buffer)               vop_cmd(_LS_NETCMD ,0x00 ,buffer ,sizeof(*buffer))
#define vop_setv4(buffer)               vop_cmd(_LS_NETCMD ,0x01 ,buffer ,sizeof(*buffer))
#define vop_getv6(buffer)               vop_cmd(_LS_NETCMD ,0x02 ,buffer ,sizeof(*buffer))
#define vop_setv6(buffer)               vop_cmd(_LS_NETCMD ,0x03 ,buffer ,sizeof(*buffer))

#define     MAGIC_NUM           0x1A2B3C4D
#define change_32bit_edian(x) (((x) << 24 & 0xff000000) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | (((x) >> 24) & 0xff))
#define change_16bit_edian(x) (((x) << 8) & 0xff00 | ((x) >> 8) & 0x00ff)
static const unsigned char INIT_VALUE = 0xfe;

int VopProtocol::vop_cmd(int cmd ,int sub_cmd, void* data ,int data_size)
{
    int direct=0 ,data_buffer_size=0;
    int real_sub_cmd = sub_cmd;
    if(cmd == _LS_WIFICMD  && sub_cmd == 0xff)
        real_sub_cmd = 0x01;
    int err = vop_getCmdDirect(cmd ,real_sub_cmd ,direct ,data_buffer_size);
    if(err)
        return ERR_vop_cannot_support;//not support

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
    ppkg->subcmd = real_sub_cmd;
    if(data){
        device_manager->mutex_ctrl.lock();
        memcpy(buffer + sizeof(COMM_HEADER) ,data ,data_size);
        device_manager->mutex_ctrl.unlock();
    }

    if(cmd == _LS_WIFICMD  && sub_cmd == 0xff)
        err = DeviceContrl::device_write_no_Read(buffer ,sizeof(COMM_HEADER)+data_buffer_size * direct);
    else
        err = DeviceContrl::device_writeThenRead(buffer ,sizeof(COMM_HEADER)+data_buffer_size * direct
                                               ,buffer ,sizeof(COMM_HEADER)+data_buffer_size * (1 - direct));
    //check result
    if(!err && MAGIC_NUM == ppkg->magic){//ACK
        if(!direct){//get
            if(data){
                device_manager->mutex_ctrl.lock();
                memcpy(data ,buffer + sizeof(COMM_HEADER) ,data_size);
                device_manager->mutex_ctrl.unlock();
            }
        }else{//set
            if(cmd == _LS_WIFICMD  && sub_cmd == 0xff)
                err = ERR_WIFI_SET_SSID;
            else
                err = ppkg->subcmd;
        }
    }else
        err = -1;
    delete buffer;
    return err;
}

void VopProtocol::copy_set_defaultPara(copycmdset* p)
{    
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(p ,&default_copy_parameter ,sizeof(default_copy_parameter));
}

void VopProtocol::copy_set_para(copycmdset* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(copy_parameter ,p ,sizeof(*p));
}

copycmdset VopProtocol::copy_get_para()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *copy_parameter;
}

void VopProtocol::wifi_set_ssid(cmdst_wifi_get* p ,const char* ssid)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    if(strlen(ssid) >= 32)
        memcpy(p->ssid ,ssid ,32);
    else{
        memset(p->ssid ,0 ,32);
        strcpy(p->ssid ,ssid);
    }
}

void VopProtocol::wifi_set_password(cmdst_wifi_get* p ,const char* password)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    if(strlen(password) >= 64)
        memcpy(p->pwd ,password ,64);
    else{
        memset(p->pwd ,0 ,64);
        strcpy(p->pwd ,password);
    }
}

void VopProtocol::wifi_set_para(cmdst_wifi_get* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(wifi_parameter ,p ,sizeof(*p));
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

cmdst_wifi_status VopProtocol::wifi_getWifiStatus()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *wifi_status;
}

cmdst_tonerEnd VopProtocol::printer_getTonerEnd()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *tonerEnd;
}

void VopProtocol::printer_setTonerEnd(cmdst_tonerEnd* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(tonerEnd ,p ,sizeof(*p));
}

cmdst_PSave_time VopProtocol::printer_getPSaveTime()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *pSaveTime;
}

void VopProtocol::printer_setPSaveTime(cmdst_PSave_time* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(pSaveTime ,p ,sizeof(*p));
}

cmdst_powerOff_time VopProtocol::printer_getPowerOffTime()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *powerOffTime;
}

void VopProtocol::printer_setPowerOffTime(cmdst_powerOff_time* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(powerOffTime ,p ,sizeof(*p));
}

void VopProtocol::passwd_set(const char* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    if(strlen(p) >= 32)
        memcpy(passwd->passwd ,p ,32);
    else{
        memset(passwd ,0 ,32);
        strcpy(passwd->passwd ,p);
    }
}

net_info_st VopProtocol::net_getV4()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *ip_info;
}

void VopProtocol::net_setV4(net_info_st* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(ip_info ,p ,sizeof(*p));
}

net_ipv6_st VopProtocol::net_getV6()
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    return *ipv6_info;
}

void VopProtocol::net_setV6(net_ipv6_st* p)
{
    QMutexLocker locker(&device_manager->mutex_ctrl);
    memcpy(ipv6_info ,p ,sizeof(*p));
}

static const char* get_cmd_string(int cmd)
{
    switch(cmd){
    case VopProtocol::CMD_GetStatus:        return "get device id";
    case VopProtocol::CMD_GetCopy:        return "copy get";
    case VopProtocol::CMD_COPY:        return "copy set";
    case VopProtocol::CMD_WIFI_apply:        return "apply wifi";
    case VopProtocol::CMD_WIFI_apply_noread:        return "wifi apply wifi";
    case VopProtocol::CMD_WIFI_get:        return "wifi get";
    case VopProtocol::CMD_WIFI_getAplist:        return "get wifi aplist";
    case VopProtocol::CMD_PASSWD_set:        return "password set";
    case VopProtocol::CMD_PASSWD_get:        return "password get";
    case VopProtocol::CMD_PASSWD_confirm:        return "password confirm";
    case VopProtocol::CMD_WIFI_GetWifiStatus:        return "wifi get status";
    case VopProtocol::CMD_PRN_TonerEnd_Get:        return "tonerend get";
    case VopProtocol::CMD_PRN_TonerEnd_Set:        return "tonerend set";
    case VopProtocol::CMD_PRN_PSaveTime_Get:        return "psave time get";
    case VopProtocol::CMD_PRN_PSaveTime_Set:        return "psave time set";
    case VopProtocol::CMD_PRN_PowerOff_Get:        return "power off get";
    case VopProtocol::CMD_PRN_PowerOff_Set:        return "power off set";
    case VopProtocol::CMD_NET_GetV4:        return "net get v4";
    case VopProtocol::CMD_NET_SetV4:        return "net set v4";
    case VopProtocol::CMD_NET_GetV6:        return "net get v6";
    case VopProtocol::CMD_NET_SetV6:        return "net set v6";
    default:return "none";
    }
}

int VopProtocol::cmd(int _cmd)
{
    int err = ERR_vop_cannot_support;
    C_LOG("exec protocol cmd:%s" ,get_cmd_string(_cmd));
    switch(_cmd)    {
    case CMD_GetStatus:{
        char buffer[1024];
        memset(buffer ,0 ,sizeof(buffer));
        err = DeviceContrl::device_getDeviceStatus(buffer ,sizeof(buffer));
        buffer[1023] = 0;//make sure buffer is a c string.
//        C_LOG("buffer size:%lu" ,strlen(buffer));
        if(!err){
            err = getStatusFromDeviceID(buffer);
        }
        break;
    }
    case CMD_GetCopy:{
        err = vop_getCopy(copy_parameter);
    }
    case CMD_COPY:{
        err = vop_copy(copy_parameter);
    }
        break;
    case CMD_WIFI_apply:{
        err =  vop_setWifiInfo(wifi_parameter);
    }
        break;
    case CMD_WIFI_apply_noread:{
        err = vop_setWifiInfo_noread(wifi_parameter);
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
    case CMD_WIFI_GetWifiStatus:{
        err = vop_getWifiStatus(wifi_status);
    }
        break;
    case CMD_PRN_TonerEnd_Get:{
        err = vop_getTonerEnd(tonerEnd);
    }
        break;
    case CMD_PRN_TonerEnd_Set:{
        err = vop_setTonerEnd(tonerEnd);
    }
        break;
    case CMD_PRN_PSaveTime_Get:{
        err = vop_getPsaveTime(pSaveTime);
    }
        break;
    case CMD_PRN_PSaveTime_Set:{
        err = vop_setPsaveTime(pSaveTime);
    }
        break;
    case CMD_PRN_PowerOff_Get:{
        err = vop_getPowerOff(powerOffTime);
    }
        break;
    case CMD_PRN_PowerOff_Set:{
        err = vop_setPowerOff(powerOffTime);
    }
        break;
        case CMD_NET_GetV4:{
        err = vop_getv4(ip_info);
    }
        break;
        case CMD_NET_SetV4:{
        err = vop_setv4(ip_info);
    }
        break;
        case CMD_NET_GetV6:{
        err = vop_getv6(ipv6_info);
    }
        break;
        case CMD_NET_SetV6:{
        err = vop_setv6(ipv6_info);
    }
        break;
    default:
        break;
    }
    return err;
}
