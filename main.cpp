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
#endif


MainWindow* gMainWidow;

QLocalServer* m_localServer;
bool isRunning(const QString& serverName)
{
    bool running = true;
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (!socket.waitForConnected()) {
        m_localServer = new QLocalServer(qApp);
        if (!m_localServer->listen(serverName)) {
            if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
                    && QFile::exists(serverName)) { //make sure listening success
                QFile::remove(serverName);
                m_localServer->listen(serverName);
            }
        }
        running = false;
    }
    return running;
}

int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
    qputenv("LIBOVERLAY_SCROLLBAR", 0);
#endif
//    qputenv("LANG" ,QLocale::system().uiLanguages().first().toLatin1());
//    qputenv("LANGUAGE" ,QLocale::system().name().toLatin1());
    QApplication a(argc, argv);
    if(isRunning("/tmp/lock_Ricoh_Alto_VOP")){
        QMessageBox::warning(0,"Warnning" ,"The application is running!");
        return 0;
    }
    Log::init();

    QTranslator trans;
//    if(!trans.load(QLocale(QLocale::system().uiLanguages().first()), "vop", ".", ":/translations"))
    if(!trans.load(QLocale(QLocale::system().name()), "vop", ".", ":/translations"))
        trans.load(QLocale(QLocale::English), "vop", ".", ":/translations");
    a.installTranslator(&trans);

    QFile file(":/styles/default.qss");
    if(file.open(QFile::ReadOnly)){
        QString stylesheet = file.readAll();
        a.setStyleSheet(stylesheet);
        file.close();
    }

    MainWindow w;
    gMainWidow = &w;
//    gMainWidow = new MainWindow;
    gMainWidow->refresh();
    gMainWidow->show();
    int ret = a.exec();
//    delete gMainWidow;
    return ret;
}
