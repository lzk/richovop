#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWidget = new MainWidget;
    setCentralWidget(mainWidget);
    ui->mainToolBar->hide();
    statusBar()->hide();
    setFixedSize(880,630);
}

MainWindow::~MainWindow()
{
    delete ui;
}
