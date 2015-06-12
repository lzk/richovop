/////////////////////////////////////////
/// File:mainwindow.h
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MainWidget* mainWidget;
};

#endif // MAINWINDOW_H
