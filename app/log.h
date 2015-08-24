/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef LOG_H
#define LOG_H

#include <QString>

#define qLog1(x)  Log::logout(x)
//#define qLog1(x)

//#include <QDebug>
//#define qLog qDebug
#define qLog(x) qLog1(x)
class Log
{
public:
    Log();
    ~Log();
    static void  init();
    static void logout(const QString&);
};

#endif // LOG_H
