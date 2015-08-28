/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwidget.h"
#include <QDesktopWidget>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWidget = new MainWidget(this);

    QScrollArea* sa = new QScrollArea(this);
    sa->setWidget(mainWidget);
    mainWidget->setFixedSize(920,650);
    setCentralWidget(sa);
//    setCentralWidget(mainWidget);
    ui->mainToolBar->hide();
    statusBar()->hide();
    slots_desktopResized(-1);
   connect(QApplication::desktop() ,SIGNAL(resized(int)) ,this ,SLOT(slots_desktopResized(int)));
    connect(ui->actionExit_Lenovo_Virtual_Panel ,SIGNAL(triggered()) ,this ,SLOT(slots_exit()));
    menuBar()->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//#include <QMessageBox>
void MainWindow::slots_exit()
{
    qApp->quit();
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

#if 0
#include <QCloseEvent>
void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    slots_exit();
//    ui->actionExit_Lenovo_Virtual_Panel->trigger();
}
#endif

void MainWindow::slots_desktopResized(int )
{
    QDesktopWidget *dwsktopwidget = QApplication::desktop();
    QRect screenrect = dwsktopwidget->screenGeometry();

    if(screenrect.width() <=  800 || screenrect.height() <= 600){
        setMinimumSize(screenrect.size() - QSize(400 ,300));
        setMaximumSize(screenrect.size() - QSize(200 ,100));
        resize(screenrect.size() - QSize(300 ,200));
    }else{
//        setMaximumSize(926,660);
        setFixedSize(926,660);
//        resize(926,660);
    }
    move((screenrect.width() - width())/2,
         (screenrect.size().height() - height())/2);
    update();
}
