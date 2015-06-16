/////////////////////////////////////////
/// File:mainwindow.cpp
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
    QMessageBox msgBox;
    msgBox.setText(tr("<h3>Lenovo Virtual Panel</h3>"));
    msgBox.setInformativeText(tr("Exit?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setWindowFlags(Qt::FramelessWindowHint);
    msgBox.setFixedSize(600 ,400);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Ok:
        exit(0);
        break;
    case QMessageBox::Cancel:
        // Cancel was clicked
        break;
    default:
        // should never be reached
        break;
  }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    ui->actionExit_Lenovo_Virtual_Panel->trigger();
}
