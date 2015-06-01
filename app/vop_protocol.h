#ifndef VOP_PROTOCOL
#define VOP_PROTOCOL

#pragma pack(1)

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;

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

void vop_setDeviceName(const char* _device_name);
int vop_copy(copycmdset* pcopycmd);

#endif // VOP_PROTOCOL

