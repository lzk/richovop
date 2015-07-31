/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef LOG_H
#define LOG_H

#include <QDebug>

#define qLog qDebug

class Log
{
public:
    Log();
    ~Log();
    static void  init();

    QDebug debug() const;
};

#endif // LOG_H
