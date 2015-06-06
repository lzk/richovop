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
    switch(cmd)
    {
    case _LS_CPYCMD:        direct = sub_cmd;        break;
    case _LS_WIFICMD:
        switch(sub_cmd)
        {
        case 0:
        case 1:            direct = sub_cmd;            break;
        case 0x07:            direct = 0;            break;
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
    case _LS_PRNCMD:
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

int vop_cmd(CMD_ID cmd ,int sub_cmd, void* data ,int data_size)
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
    result = DeviceManager::device_writeThenRead(buffer ,sizeof(COMM_HEADER)+data_size * direct
                                               ,buffer ,sizeof(COMM_HEADER)+data_size * direct);
    if(!result && MAGIC_NUM == ppkg->magic)
    {
        result = ppkg->subcmd;
    }
    return result;
}


