#include <sys/statfs.h>
#include "log.h"


#define MAX_SPACE 100

bool device_no_space(const char* path)
{
    struct statfs tmp_buf;
    unsigned int t_space_size = MAX_SPACE;
    if(!statfs(path ,&tmp_buf)){
        t_space_size = (unsigned int)(tmp_buf.f_bavail * tmp_buf.f_bsize >> 10);
    }
    qLog(QString().sprintf("device space:%d K" ,t_space_size));
    return t_space_size < MAX_SPACE;
}
