/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwidget.h"
#include <QDesktopWidget>
#include <QScrollArea>
#include <QMdiArea>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

//    Qt::WindowFlags wf = 0;
//    wf |= Qt::WindowCloseButtonHint ;
//    setWindowFlags(wf);
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
    int ret = messagebox_exec(tr("IDS_EXIT_CONFIRM")+"                                    ",QMessageBox::Ok | QMessageBox::Cancel ,QMessageBox::Ok);
//    int ret = mainWidget->messagebox_exec(tr("IDS_Menu_Exit")+"?" ,QMessageBox::Ok | QMessageBox::Cancel ,QMessageBox::Ok);
    switch (ret) {
    case QMessageBox::Ok:
        event->accept();
        qApp->quit();
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

QMessageBox::StandardButton MainWindow::messagebox_exec(const QString &text,
                                                QMessageBox::StandardButtons buttons,
                                               QMessageBox::StandardButton defaultButton)
{
    QMessageBox msgbox;
    QMessageBox* mb = & msgbox;
    mb->setWindowIcon(QIcon(":/images/printer.ico"));
    mb->setWindowTitle("RICOH Printer");
    mb->setText(text);
    mb->setIconPixmap(QPixmap(":/images/printer.ico"));

//    mb->setText(title);
//    mb->setInformativeText(text);

    mb->setStandardButtons(buttons);
    mb->setDefaultButton(defaultButton);
//    mb->setWindowFlags(Qt::FramelessWindowHint);
#if 1
    mb->show();//show first before get real size
//    QPoint widget_pos = mapToGlobal(pos());
//    mb->move(widget_pos.x() + (width() - mb->width())/2,
//         widget_pos.y() + (height() - mb->height())/2 - 50);
    mb->move((QApplication::desktop()->width() - mb->width())/2,
          (QApplication::desktop()->height() - mb->height())/2);
//    mb->raise();
//    mb->activateWindow();
#endif
    return (QMessageBox::StandardButton)mb->exec();
}
