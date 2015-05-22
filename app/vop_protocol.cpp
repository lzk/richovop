#include <string.h>
#include "vop_protocol.h"

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

#define     MAGIC_NUM           0x1A2B3C4D
static const unsigned char INIT_VALUE = 0xfe;

extern int device_cmd(char* buffer ,int len);
static int vop_cmd(CMD_ID cmd ,int sub_cmd, void* data ,int data_size)
{
    int result = 0;
    int len = 0;
    switch(cmd)
    {
    case _LS_CPYCMD:len=128;break;
    default: break;
    }

    int device_cmd_len = sizeof(COMM_HEADER)+len;
    char* buffer = new char[device_cmd_len];
    memset( buffer, INIT_VALUE, sizeof(device_cmd_len));
    COMM_HEADER* ppkg = reinterpret_cast<COMM_HEADER*>( buffer );

    ppkg->magic = MAGIC_NUM ;
    ppkg->id = _LS_CPYCMD;
    ppkg->len = 3+len;

    // For the simple data setting, e.g. copy/scan/prn/wifi/net, SubID is always 0x13, len is always 0x01,
    // it just stand for the sub id. The real data length is defined by the lib
    ppkg->subid = 0x13; // copy set command
    ppkg->len2 = 1;
    ppkg->subcmd = sub_cmd;   // set copy command

    memcpy(buffer + sizeof(COMM_HEADER) ,data ,data_size);
//    result = device_cmd(buffer ,device_cmd_len);
    return result;
}

int vop_copy(copycmdset* pcopycmd)
{
    return vop_cmd(_LS_CPYCMD ,1 ,pcopycmd ,sizeof(copycmdset));
}
