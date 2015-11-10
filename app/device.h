#ifndef DEVICE_H
#define DEVICE_H


class Device
{
public:
    Device();
    virtual ~Device();

    enum{
        Device_invalid = 0,
        Device_3in1 = 1,
        Device_3in1_wifi = 2,
        Device_sfp = 3,
        Device_sfp_wifi = 4,

    };
    virtual void init(char*) = 0;
    virtual int openPrinter(const char*) = 0;
    virtual int openPrinter(void) = 0;
    virtual int closePrinter(void) = 0;
    virtual int write(char *buffer, int bufsize) = 0;
    virtual int read(char *buffer, int bufsize) = 0;
    virtual int get_device_id(char *buffer, int bufsize) = 0;
    virtual int get_driver() = 0;
    virtual int attach_driver() = 0;
    int write_then_read(char* wrBuffer ,int wrSize ,char* rdBuffer ,int rdSize);

    static int getDeviceModel(const char* printer_info);
protected:
    virtual void delay(int) = 0;
    void* hLLD;
};

#endif // DEVICE_H
