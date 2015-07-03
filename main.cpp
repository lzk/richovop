/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwindow.h"
#include <QApplication>
#include<QTranslator>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator trans;
    QString filename =":/translations/vop_" + QLocale::system().name();
    trans.load(filename);
    a.installTranslator(&trans);

    MainWindow w;
    w.show();

    return a.exec();
}
