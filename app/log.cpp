#include "log.h"
#include <qapplication.h>
#include <stdio.h>
//#include <stdlib.h>
#include <QFile>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
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
    QFile file("vop.log");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream out(&file);
    out << msg << endl;
    file.close();
}


Log log;
Log::Log()
{
    qInstallMessageHandler(myMessageOutput);
}

Log::~Log()
{
}

QDebug Log::debug() const
{
    return qDebug();
}

