/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef LOG_H
#define LOG_H

#include <QDebug>

#define qLog qDebug


class Log;
extern Log log;
//#define qLog log.debug
class Log
{
public:
    Log();
    ~Log();

    QDebug debug() const;
};

#endif // LOG_H
