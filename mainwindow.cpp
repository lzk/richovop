/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwidget.h"
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWidget = new MainWidget(this);
    setCentralWidget(mainWidget);
    ui->mainToolBar->hide();
    statusBar()->hide();
    setFixedSize(926,660);
    move((QApplication::desktop()->width() - width())/2,
         (QApplication::desktop()->height() - height())/2);

    connect(ui->actionExit_Lenovo_Virtual_Panel ,SIGNAL(triggered()) ,this ,SLOT(slots_exit()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
#include <QMessageBox>
void MainWindow::slots_exit()
{
    exit(0);
//    int ret = mainWidget->messagebox_exec(tr("Exit")+"?" ,QMessageBox::Ok | QMessageBox::Cancel ,QMessageBox::Ok);
//    switch (ret) {
//    case QMessageBox::Ok:
//        exit(0);
//        break;
//    case QMessageBox::Cancel:
//        // Cancel was clicked
//        break;
//    default:
//        // should never be reached
//        break;
//  }
}

#include <QCloseEvent>
void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    slots_exit();
//    ui->actionExit_Lenovo_Virtual_Panel->trigger();
}
