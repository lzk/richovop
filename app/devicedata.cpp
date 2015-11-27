#include "devicedata.h"
#include <string.h>

DeviceData::DeviceData()
{
    clear();
}

DeviceData::~DeviceData()
{

}

void DeviceData::clear()
{
    memset(&copy_data ,0 ,sizeof(copy_data));
}


struct CopyData* DeviceData::get_copy_data()
{
    return &copy_data;
}
