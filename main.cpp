/////////////////////////////////////////
/// File:main.cpp
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
