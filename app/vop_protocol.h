/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef VOP_PROTOCOL
#define VOP_PROTOCOL

#pragma pack(1)

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct
{
    ////////////////////////////////////////////////////
    // Consumable
    ////////////////////////////////////////////////////
    BYTE	TonelStatusLevelK;
    BYTE	TonelStatusLevelC;
    BYTE	TonelStatusLevelM;
    BYTE	TonelStatusLevelY;
    BYTE	DrumStatusLifeRemain;

    ////////////////////////////////////////////////////
    // Covers
    ////////////////////////////////////////////////////
    BYTE	CoverStatusFlags;

    ////////////////////////////////////////////////////
    // Paper Tray
    ////////////////////////////////////////////////////
    BYTE	PaperTrayStatus;
    BYTE	PaperSize;

    ////////////////////////////////////////////////////
    // Output Tray
    ////////////////////////////////////////////////////
    BYTE	OutputTrayLevel;

    ////////////////////////////////////////////////////
    // General Status and information
    ////////////////////////////////////////////////////
    BYTE	PrinterStatus;
    WORD	OwnerName[16];
    WORD	DocuName[16];
    BYTE	ErrorCodeGroup;
    BYTE	ErrorCodeID;
    WORD	PrintingPage;
    WORD	Copies;
    DWORD	TotalCounter;

    BYTE         reserved[12];

    BYTE	TonerSize[4];
    BYTE	PaperType;
    BYTE	NonDellTonerMode;
    BYTE	AioStatus;
    BYTE	job;
    WORD	wReserved1;
    WORD	wReserved2;
} PRINTER_STATUS;

enum{
    ERR_ACK = 0,
    ERR_CMD_invalid = 1,
    ERR_Parameter_invalid = 2,
    ERR_Do_not_support = 3,
    ERR_Printer_busy = 4,
    ERR_Printer_error = 5,
    ERR_Set_parameter_error = 6,
    ERR_Get_parameter_error = 7,
    ERR_Printer_is_Sleeping = 8,
    ERR_Printer_is_in_error = 9,
    ERR_Password_incorrect = 0x0a,
    ERR_Scanner_operation_NG =0x0b,

    //err self defined
    ERR_communication = -1,
    ERR_library = -2,
    ERR_decode_status = -3,
    ERR_wifi_have_not_been_inited = -4,
    ERR_decode_device = -5,
    ERR_vop_cannot_support = -10,

    //status self defined
    STATUS_ready= -100,
    STATUS_sleep,
    STATUS_busy_printing,
    STATUS_busy_scanningOrCoping,
    STATUS_jam,
    STATUS_CopyScanNextPage,
    STATUS_TonerEnd,
    STATUS_no_defined,
};

typedef struct _copycmdset
{
        UINT8 Density         ; // 0  -   0~6
        UINT8 copyNum         ; // 1  -   1~99
        UINT8 scanMode        ; // 2  -   0: Photo, 1: Text, 2: ID card
        UINT8 orgSize         ; // 3  -   0: A4, 1: A5, 2: B5, 3: Letter, 4: Executive
        UINT8 paperSize       ; // 4  -   0: Letter, 1: A4, 2: A5, 3: A6, 4: B5, 5: B6, 6: Executive, 7: 16K
        UINT8 nUp             ; // 5  -   0:1up, 1: 2up, 3: 4up, 4: 9up
        UINT8 dpi             ; // 6  -   0: 300*300, 1: 600*600
        UINT8 mediaType       ; // 7  -   0: plain paper 1: Recycled paper 2: Thick paper 3: Thin paper 4: Label
        UINT16 scale          ; // 8  -   25~400, disabled for 2/4/9up
} copycmdset;

typedef struct cmdst_wifi_get
{
    UINT8 wifiEnable ; // bit0: Wi-Fi Enable, bit1: GO Enable, bit2: P2P Enable
    UINT8 sigLevel   ; //
    UINT8 reserved0  ; //
    UINT8 netType    ; // lenovo always 0
    UINT8 encryption ; // 0:No Security 1:WEP 64/128 bit 2.WPA-PSK-TKIP  3. WPA2-PSK-AES 4.Mixed Mode PSK
    UINT8 wepKeyId   ; //
    UINT8 reserved1  ; //
    UINT8 channel    ; //
    char  ssid[32]   ; // used by both Legacy WiFi SSID and Wi-Fi Direct GO SSID
    char  pwd[64]    ; // used by both Legacy WiFi Passphrase & WEPKey and Wi-Fi Direct GO Passphrase
    char  pinCode[8] ; //
    UINT8 reserved2[64]; //
    UINT8 ipAddr[4]  ; //
}cmdst_wifi_get;

typedef struct cmdst_wifi_set
{
    UINT8 wifiEnable ; // bit0: Wi-Fi Enable, bit1: GO Enable, bit2: P2P Enable
    UINT8 reserved0[2]; //
    UINT8 netType    ; //
    UINT8 encryption ; //
    UINT8 wepKeyId   ; //
    UINT8 reserved1[2]; //
    char  ssid[32]   ; // used by both Legacy WiFi SSID and Wi-Fi Direct GO SSID
    char  pwd[64]    ; // used by both Legacy WiFi Passphrase & WEPKey and Wi-Fi Direct GO Passphrase
}cmdst_wifi_set;

struct cmdst_aplist
{
    char ssid[33];
    UINT8 encryption;
};

#define NUM_OF_APLIST 10
typedef struct cmdst_aplist_get
{
    struct cmdst_aplist  aplist[NUM_OF_APLIST];
}
    cmdst_aplist_get;

typedef struct cmdst_passwd
{
    char passwd[32];
}
    cmdst_passwd;

typedef UINT8  cmdst_wifi_status;
typedef UINT8  cmdst_tonerEnd;
typedef UINT8  cmdst_PSave_time;
typedef UINT8  cmdst_powerOff_time;

typedef struct net_info_st
{
    UINT8 IPMode            ; // 0-ipv4,1-ipv6
    UINT8 IPAddressMode     ; // 0 AutoIP,1 BOOTP,2 RARP,3 DHCP,4 Panel (Manual)
    UINT8 IPAddress[4]      ; // 0.0.0.0 ~ 223.255.255.255
    UINT8 SubnetMask[4]     ; // 0.0.0.0 ~ 223.255.255.255
    UINT8 GatewayAddress[4] ; // 0.0.0.0 ~ 223.255.255.255
} net_info_st;

typedef struct
{
    UINT8 UseManualAddress;
    char ManualAddress[40];
    UINT32 ManualMask;
    char StatelessAddress1[40];
    char StatelessAddress2[40];
    char StatelessAddress3[40];
    char LinkLocalAddress[40];
    char IPv6ManualGatewayAddress[40];
    char AutoGatewayAddress[40];
    char AutoStatefulAddress[40];
    UINT8 DHCPv6;
}net_ipv6_st;

enum
{
    PSTATUS_Ready                      = 0x00,
    PSTATUS_Printing                   = 0x01,
    PSTATUS_PowerSaving                = 0x02,
    PSTATUS_WarmingUp                  = 0x03,
    PSTATUS_PrintCanceling             = 0x04,
    PSTATUS_Processing                 = 0x07,
    PSTATUS_CopyScanning               = 0x60,
    PSTATUS_CopyScanNextPage           = 0x61,
    PSTATUS_CopyPrinting               = 0x62,
    PSTATUS_CopyCanceling              = 0x63,
    PSTATUS_IDCardMode                 = 0x64,
    PSTATUS_ScanScanning               = 0x6A,
    PSTATUS_ScanSending                = 0x6B,
    PSTATUS_ScanCanceling              = 0x6C,
    PSTATUS_ScannerBusy                = 0x6D,
    PSTATUS_TonerEnd1                  = 0x7F,//For china maket
    PSTATUS_TonerEnd2                  = 0x80,
    PSTATUS_TonerNearEnd               = 0x81,
    PSTATUS_ManualFeedRequired         = 0x85,
    PSTATUS_InitializeJam              = 0xBC,
    PSTATUS_NofeedJam                  = 0xBD,
    PSTATUS_JamAtRegistStayOn          = 0xBE,
    PSTATUS_JamAtExitNotReach          = 0xBF,
    PSTATUS_JamAtExitStayOn            = 0xC0,
    PSTATUS_CoverOpen                  = 0xC1,
    PSTATUS_NoTonerCartridge           = 0xC5,
    PSTATUS_WasteTonerFull             = 0xC6,
    PSTATUS_FWUpdate                   = 0xC7,
    PSTATUS_OverHeat                   = 0xC8,
    PSTATUS_PolygomotorOnTimeoutError  = 0xCD,
    PSTATUS_PSTATUS_PolygomotorOffTimeoutError = 0xCE,
    PSTATUS_PolygomotorLockSignalError = 0xCF,
    PSTATUS_BeamSynchronizeError       = 0xD1,
    PSTATUS_BiasLeak                   = 0xD2,
    PSTATUS_PlateActionError           = 0xD3,
    PSTATUS_MainmotorError             = 0xD4,
    PSTATUS_MainFanMotorEorror         = 0xD5,
    PSTATUS_JoinerThermistorError       = 0xD6,
    PSTATUS_JoinerReloadError           = 0xD7,
    PSTATUS_HighTemperatureErrorSoft   = 0xD8,
    PSTATUS_HighTemperatureErrorHard   = 0xD9,
    PSTATUS_JoinerFullHeaterError       = 0xDA,
    PSTATUS_Joiner3timesJamError        = 0xDB,
    PSTATUS_LowVoltageJoinerReloadError = 0xDC,
    PSTATUS_MotorThermistorError       = 0xDD,
    PSTATUS_EEPROMCommunicationError   = 0xDE,
    PSTATUS_CTL_PRREQ_NSignalNoCome    = 0xDF,
    PSTATUS_ScanPCUnkownCommandUSB     = 0xE0,
    PSTATUS_SCANUSBDisconnect          = 0xE1,
    PSTATUS_ScanPCUnkownCommandNET     = 0xE3,
    PSTATUS_ScanNETDisconnect          = 0xE4,
    PSTATUS_ScanMotorError             = 0xE5,
    PSTATUS_NetWirelessConnectFail     = 0xE6,
    PSTATUS_NetWirelessDisable         = 0xE7,
    PSTATUS_NetWirelessDongleCfgFail   = 0xE8,
    PSTATUS_FWUpdateError              = 0xEB,
    PSTATUS_DSPError                   = 0xEC,
    PSTATUS_CodecError                 = 0xED,
    PSTATUS_PrinterDataError           = 0xEF,
    PSTATUS_Unknown                    = 0xF0, // status added by SW
    PSTATUS_Offline                    = 0xF1, // status added by SW
    PSTATUS_PowerOff                   = 0xF2, // status added by SW
};

class DeviceManager;
class VopProtocol
{
public:
    enum{
        CMD_GetStatus,
        CMD_GetCopy,
        CMD_COPY,
        CMD_WIFI_apply,
        CMD_WIFI_get,
        CMD_WIFI_getAplist,
        CMD_PASSWD_set,
        CMD_PASSWD_get,
        CMD_PASSWD_confirm,
        CMD_WIFI_GetWifiStatus,
        CMD_PRN_TonerEnd_Get,
        CMD_PRN_TonerEnd_Set,
        CMD_PRN_PSaveTime_Get,
        CMD_PRN_PSaveTime_Set,
        CMD_PRN_PowerOff_Get,
        CMD_PRN_PowerOff_Set,
        CMD_NET_GetV4,
        CMD_NET_SetV4,
        CMD_NET_GetV6,
        CMD_NET_SetV6,
    };
public:
    VopProtocol(DeviceManager* dm);
    ~VopProtocol();
    static const char* getErrString(int err);
    static int DecodeStatusFromDeviceID(char* device_id, PRINTER_STATUS* status);
    static int getDESfromDeviceID(char* device_id ,char*);
    int getStatusFromDeviceID(char* device_id);

    PRINTER_STATUS get_status();
    int get_deviceStatus();

    void copy_set_defaultPara(copycmdset* p);
    void copy_set_para(copycmdset* p);
    copycmdset copy_get_para();
    void wifi_set_ssid(cmdst_wifi_get*  ,const char*);
    void wifi_set_password(cmdst_wifi_get*  ,const char*);
    void wifi_set_para(cmdst_wifi_get* p);
    cmdst_wifi_get wifi_get_para();
    cmdst_aplist_get wifi_getAplist();
    cmdst_wifi_status wifi_getWifiStatus();
    cmdst_tonerEnd printer_getTonerEnd();
    void printer_setTonerEnd(cmdst_tonerEnd*);
    cmdst_PSave_time printer_getPSaveTime();
    void printer_setPSaveTime(cmdst_PSave_time*);
    cmdst_powerOff_time printer_getPowerOffTime();
    void printer_setPowerOffTime(cmdst_powerOff_time*);
    net_info_st net_getV4();
    void net_setV4(net_info_st*);
    net_ipv6_st net_getV6();
    void net_setV6(net_ipv6_st*);

    void passwd_set(const char*);

    int cmd(int);
private:
    DeviceManager* device_manager;

    PRINTER_STATUS* status;
    copycmdset* copy_parameter;
    cmdst_wifi_get* wifi_parameter;
    cmdst_aplist_get* wifi_aplist;
    cmdst_passwd* passwd;
    cmdst_wifi_status* wifi_status;
    cmdst_tonerEnd* tonerEnd;
    cmdst_PSave_time* pSaveTime;
    cmdst_powerOff_time* powerOffTime;
    net_info_st* ip_info;
    net_ipv6_st* ipv6_info;

    int vop_cmd(int cmd ,int sub_cmd, void* data ,int data_size);
# if 0
    int cmd_copy();
    int cmd_wifi_set();
    int cmd_wifi_get();
    int cmd_wifi_getAplist();
    int cmd_passwd_set();
    int cmd_passwd_get();
    int cmd_passwd_confirm();
#endif
};
#endif // VOP_PROTOCOL

