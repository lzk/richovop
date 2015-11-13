/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "log.h"
#include <qapplication.h>
#include <stdio.h>
//#include <stdlib.h>
#include <QFile>
#include <QMutex>
#include <QTime>
#include <QThread>
#include<QTextStream>
static QMutex mutex_write_log_file;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    (void)context;
#else
void myMessageOutput(QtMsgType type, const char *msg)
{
#endif
    (void)type;
#if 0
    QByteArray localMsg = msg.toLocal8Bit();
    char str[256];
    memset(str ,0 ,256);
    switch (type) {
    case QtDebugMsg:
        sprintf(str, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        sprintf(str, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        sprintf(str, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        sprintf(str, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
#endif
    QMutexLocker locker(&mutex_write_log_file);
    QFile file("/tmp/AltoVOP.log");
    if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz:")
            <<QThread::currentThreadId()<<"                        " <<msg << endl;
        file.close();
    }

//    QString str;
//    str.sprintf("echo %s >> /tmp/AltoVOP.log" ,msg);
//    system(str.toLatin1());
}

Log::Log()
{
}

Log::~Log()
{
}

#include "../version.h"
void  Log::init()
{
//    system("echo \"----------------AltoVOP debug log------------------\" > /tmp/AltoVOP.log");
    QFile file("/tmp/AltoVOP.log");
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        QTextStream out(&file);
        QString str;
        out << str.sprintf("----------------AltoVOP %s  %s  ------------------" ,vop_version ,copy_right) << endl;
        file.close();
    }
    (void)system("chmod 666 /tmp/AltoVOP.log 2>>/tmp/AltoVOP.log");
#if 0
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    qInstallMessageHandler(myMessageOutput);
#else
    qInstallMsgHandler(myMessageOutput);
#endif
#endif
}

void Log::logout(const QString& msg)
{
    QMutexLocker locker(&mutex_write_log_file);

    QFile file("/tmp/AltoVOP.log");
    if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz:")
            <<QThread::currentThreadId()<<"                        " <<msg << endl;
        file.close();
    }
}
