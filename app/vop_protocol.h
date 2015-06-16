/////////////////////////////////////////
/// File:vop_protocol.h
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef VOP_PROTOCOL
#define VOP_PROTOCOL

#pragma pack(1)

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
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

    ERR_NOERR = 0,
    ERR_communication = -1,
    ERR_library = -2,
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

class DeviceManager;
class VopProtocol
{
public:
    VopProtocol(DeviceManager*);
    ~VopProtocol();
    static const char* getErrString(int err);

    void copy_set_defaultPara();
    void copy_set_para(copycmdset* p);
    copycmdset copy_get_para();

    void wifi_set_ssid(cmdst_wifi_get*  ,const char*);
    void wifi_set_password(cmdst_wifi_get*  ,const char*);
    void wifi_set_para(cmdst_wifi_get* p);
    cmdst_wifi_get wifi_get_para();
    cmdst_aplist_get wifi_getAplist();

    void passwd_set(const char*);

enum{
    CMD_COPY,
    CMD_WIFI_apply,
    CMD_WIFI_get,
    CMD_WIFI_getAplist,
    CMD_PASSWD_set,
    CMD_PASSWD_get,
    CMD_PASSWD_confirm,
};
    int cmd(int);
private:
    DeviceManager* deviceManager;
    copycmdset* copy_parameter;
    cmdst_wifi_get* wifi_parameter;
    cmdst_aplist_get* wifi_aplist;
    cmdst_passwd* passwd;
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

