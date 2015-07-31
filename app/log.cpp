/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "log.h"
#include <qapplication.h>
#include <stdio.h>
//#include <stdlib.h>
#include <QFile>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
void myMessageOutput(QtMsgType type, const char *msg)
#endif
{
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
    QFile file("/tmp/AltoVOP.log");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream out(&file);
    out << msg << endl;
    file.close();
}

Log::Log()
{
}

Log::~Log()
{
}

void  Log::init()
{
#if 1
    system("echo \"----------------AltoVOP debug log------------------\" > /tmp/AltoVOP.log");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    qInstallMessageHandler(myMessageOutput);
#else
    qInstallMsgHandler(myMessageOutput);
#endif
#endif
}

QDebug Log::debug() const
{
    return qDebug();
}

