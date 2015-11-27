/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef LOG_H
#define LOG_H

#include <QString>

#if 0
class Log
{
public:
    Log();
    ~Log();
    static void  init();
    static void logout(const QString&);
};

#define _Q_LOG(x) Log::logout(x)
#define C_LOG(format, ...) Log::logout(QString().sprintf(format ,##__VA_ARGS__))
#else
#define _Q_LOG(x) logout(x)
#define C_LOG(format, ...) logout(QString().sprintf(format ,##__VA_ARGS__))
#endif
void init_log_file();
void init_log();
void logout(const QString& msg);

#endif // LOG_H
