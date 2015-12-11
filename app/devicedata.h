#ifndef DEVICEDATA_H
#define DEVICEDATA_H

struct CopyData
{
    bool status;
    bool this_copy;
};

class DeviceData
{
public:
    DeviceData();
    ~DeviceData();

    void clear();
    struct CopyData* get_copy_data();

private:
    struct CopyData copy_data;
};

#endif // DEVICEDATA_H
