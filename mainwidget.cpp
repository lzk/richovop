#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "ui_copy.h"
#include "ui_setting.h"
#include "ui_about.h"
#include "newwidget.h"

#include <QMouseEvent>
//#include <QProcess>
#include <QDebug>
#include <QAction>

#include <QDesktopServices>
#include<QUrl>

#include <QBitmap>
//#include<QPrinterInfo>

//#include "sys_sys/sys_sys.h"
MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    tc(new Ui::TabCopy),
//    tc(new NewWidget),
    ts(new Ui::TabSetting),
    ta(new Ui::TabAbout)
{
    ui->setupUi(this);
    initializeUi();
    createActions();
    retranslateUi();
//    setFixedSize(800,600);
}

MainWidget::~MainWidget()
{
    delete ui;
    delete tc;
    delete ts;
    delete ta;
}

void MainWidget::retranslateUi()
{
    setWindowTitle(tr("vop"));
} // retranslateUi

QStringList get_dests();
void MainWidget::initializeUi()
{
    tc->setupUi(ui->tab_3);
    ts->setupUi(ui->tab_4);
    ta->setupUi(ui->tab_5);

    QStringList printerNames = get_dests();
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
//    printerNames =QPrinterInfo::availablePrinterNames();
//#else
//    QList<QPrinterInfo> printerInfoList = QPrinterInfo::availablePrinters();
//    for(int i = 0 ; i < printerInfoList.count() ;i++)
//    {
//        printerNames.append(printerInfoList[i].printerName());
//    }
//#endif
    ui->comboBox_deviceList->insertItems(0 ,printerNames);
    ui->tabWidget->setBackgroundRole(QPalette::Button);

//    tc->toolButton_idCardCopy->setIconSize(QPixmap(":/images/roundIDCopy.png").size());
//    tc->toolButton_copy->setIconSize(QPixmap(":/images/squareCopyAlt.png").size());
//    tc->toolButton_idCardCopy->setIconSize(QSize(100,100));
//    tc->toolButton_copy->setIconSize(QSize(100,100));

//    ta->label->setMask(QPixmap(QString::fromUtf8(":/images/about.png")).mask());
    ta->label->installEventFilter(this);
//    ps = new QProcess(this);

}

void MainWidget::createActions()
{
    action_update = new QAction(this);
    connect(action_update ,SIGNAL(triggered()) ,this ,SLOT(slots_update()));
}

bool MainWidget::eventFilter(QObject *obj, QEvent *event)
{

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if(Qt::LeftButton == mouseEvent->button()
                && mouseEvent->x() > 130 && mouseEvent->y() > 280
                && mouseEvent->x() < 180 && mouseEvent->y() < 330
                )
        {
//            action_update->activate(QAction::Trigger);
            action_update->trigger();
            qDebug() << "pos:" << mouseEvent->pos();
        }
        return true;
    } else {
        // standard event processing
        return QWidget::eventFilter(obj, event);
    }
}

void MainWidget::slots_update()
{
//    ps->start(QLatin1String(sys_open) + "http://www.lenovo.com");
    QDesktopServices::openUrl(QUrl("http://www.lenovo.com"));
}
