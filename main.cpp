/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwindow.h"
#include <QApplication>
#include<QTranslator>
#include <QLocale>
#include <QFile>
#include "app/log.h"
#include "app/linux_api.h"
#include "version.h"

#ifdef STATIC_BUILD
#include <QtPlugin>
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qtiff)
Q_IMPORT_PLUGIN(qmng)
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qico)
#endif

MainWindow* gMainWindow;
void quit(int)
{
    _Q_LOG("SIGINT quit");
    if(qApp)
        qApp->quit();
}

bool g_region_paper_is_A4;
static bool g_system_paper_is_A4;
bool system_paper_is_A4()
{
    return g_system_paper_is_A4;
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
    a.setWindowIcon(QIcon(":/printer.png"));
//    a.setApplicationName(vop_name);
//    a.setOrganizationName(vop_name);

    QTranslator trans;
//    if(!trans.load(QLocale(QLocale::system().uiLanguages().first()), "vop", ".", ":/translations"))
    if(!trans.load(QLocale(QLocale::system().name()), "vop", ".", ":/translations"))
        trans.load(QLocale(QLocale::English), "vop", ".", ":/translations");

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name() ,":/translations");
    a.installTranslator(&trans);
    a.installTranslator(&qtTranslator);

    init_log_file();
//    if(is_running(QApplication::applicationDirPath() ,vop_name)){//QApplication::applicationName())){
    if(isRunning("/tmp/lock_Ricoh_Alto_VOP")){
        _Q_LOG("");
        _Q_LOG("another launcher is running");
        _Q_LOG("");
        MainWindow::messagebox_exec(a.translate("MainWindow" ,"IDS_ANOTHER_LAUNCHER_RUNNING"));
        return 0;
    }
    init_log();

    g_system_paper_is_A4 = region_paper_is_A4();
    g_region_paper_is_A4 = g_system_paper_is_A4;
    C_LOG("region default paper size is A4:%d" ,g_region_paper_is_A4);
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
