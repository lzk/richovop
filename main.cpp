/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwindow.h"
#include <QApplication>
#include<QTranslator>
#include <QLocale>
#include<QMessageBox>
#include "app/log.h"

#include <QLocalSocket>
 #include <QLocalServer>
#include <QFile>

#ifdef STATIC_BUILD
#include <QtPlugin>
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qtiff)
Q_IMPORT_PLUGIN(qmng)
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qico)
#endif


MainWindow* gMainWindow;
QLocalServer* m_localServer;
bool isRunning(const QString& serverName)
{
    bool running = true;
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (!socket.waitForConnected()) {
        Log::init();
        qLog(socket.errorString());
        if(QFile::exists(serverName))
            QFile::remove(serverName);

        m_localServer = new QLocalServer(qApp);
        if (!m_localServer->listen(serverName)) {
            if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
                    && QFile::exists(serverName)) { //make sure listening success
                QFile::remove(serverName);
                m_localServer->listen(serverName);
            }
        }
        (void)system(("chmod a+w " + serverName).toLatin1() + " 2>/dev/null");
        running = false;
    }
    return running;
}

void quit(int)
{
    qLog("ctrl+c quit");
    if(qApp)
        qApp->quit();
}

#include <sys/wait.h>
int main(int argc, char *argv[])
{
    signal(SIGINT ,quit);
#ifdef Q_WS_X11
    qputenv("LIBOVERLAY_SCROLLBAR", 0);
#endif
//    qputenv("LANG" ,QLocale::system().uiLanguages().first().toLatin1());
//    qputenv("LANGUAGE" ,QLocale::system().name().toLatin1());
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/printer.ico"));
//    a.setApplicationName("RICOH Printer");
//    a.setOrganizationName("RICOH Printer");

    QTranslator trans;
//    if(!trans.load(QLocale(QLocale::system().uiLanguages().first()), "vop", ".", ":/translations"))
    if(!trans.load(QLocale(QLocale::system().name()), "vop", ".", ":/translations"))
        trans.load(QLocale(QLocale::English), "vop", ".", ":/translations");

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name() ,":/translations");
    a.installTranslator(&trans);
    a.installTranslator(&qtTranslator);

    if(isRunning("/tmp/lock_Ricoh_Alto_VOP")){
        MainWindow::messagebox_exec(a.translate("MainWindow" ,"IDS_ANOTHER_LAUNCHER_RUNNING"));
        return 0;
    }

    QFile file(":/styles/default.qss");
    if(file.open(QFile::ReadOnly)){
        QString stylesheet = file.readAll();
        a.setStyleSheet(stylesheet);
        file.close();
    }

    MainWindow w;
    gMainWindow = &w;
//    gMainWindow = new MainWindow;
    gMainWindow->refresh();
    gMainWindow->show();
    int ret = a.exec();
//    delete gMainWindow;
    return ret;
}
