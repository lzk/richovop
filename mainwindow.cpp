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
//    Qt::WindowFlags wf = windowFlags();
//    wf &= ~Qt::WindowMaximizeButtonHint;
//    setWindowFlags(wf);
    mainWidget = new MainWidget;
    QScrollArea* sa = new QScrollArea;
    sa->setFocusPolicy(Qt::NoFocus);
    sa->setWidget(mainWidget);
    mainWidget->setFixedSize(920,650);
    setCentralWidget(sa);
//    setCentralWidget(mainWidget);
    ui->mainToolBar->hide();
    statusBar()->hide();
    slots_desktopResized(-1);
   connect(QApplication::desktop() ,SIGNAL(resized(int)) ,this ,SLOT(slots_desktopResized(int)));
    connect(ui->actionExit ,SIGNAL(triggered()) ,this ,SLOT(slots_exit()));
    menuBar()->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh()
{
    mainWidget->on_refresh_clicked();
}

void MainWindow::slots_exit()
{
    qApp->quit();
}

#include <QCloseEvent>
void MainWindow::closeEvent(QCloseEvent *event)
{
    int ret = QMessageBox::question(this ,tr("RICOH Printer") ,tr("IDS_Menu_Exit")+"?                                    " ,QMessageBox::Ok | QMessageBox::Cancel ,QMessageBox::Ok);
//    int ret = mainWidget->messagebox_exec(tr("IDS_Menu_Exit")+"?" ,QMessageBox::Ok | QMessageBox::Cancel ,QMessageBox::Ok);
    switch (ret) {
    case QMessageBox::Ok:
        event->accept();
        break;
    case QMessageBox::Cancel:
        event->ignore();
    default:
        break;
  }
}

void MainWindow::slots_desktopResized(int )
{
    QDesktopWidget *dwsktopwidget = QApplication::desktop();
    QRect screenrect = dwsktopwidget->screenGeometry();

    if(screenrect.width() <  926 || screenrect.height() < 660){
//        resize(screenrect.size() - QSize(300 ,200));
           setMinimumSize(400 ,300);
           resize(400 ,300);
//           showFullScreen();
    }else{
        showNormal();
//        resize(926,660);
        setFixedSize(926,660);
    }
    move((screenrect.width() - width())/2,
         (screenrect.size().height() - height())/2);
    update();
}
