#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwidget.h"
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWidget = new MainWidget;
    setCentralWidget(mainWidget);
    ui->mainToolBar->hide();
    statusBar()->hide();
    setFixedSize(926,660);
    move((QApplication::desktop()->width() - width())/2,
         (QApplication::desktop()->height() - height())/2);
}

MainWindow::~MainWindow()
{
    delete ui;
}
