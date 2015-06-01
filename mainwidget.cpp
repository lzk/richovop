#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "ui_copy.h"
#include "ui_setting.h"
#include "ui_about.h"

//#include <QProcess>
#include <QAction>
#include <QDebug>
#include <QMainWindow>

#include "app/devicemanager.h"
#include "app/vop_protocol.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    tc(new Ui::TabCopy),
    ts(new Ui::TabSetting),
    ta(new Ui::TabAbout),
    deviceManager(new DeviceManager),
    status(0)
{
    ui->setupUi(this);
    createActions();
    initializeUi();
    retranslateUi();
    initialize();
    updateUi();
}

MainWidget::~MainWidget()
{
    deviceManageThread.quit();
    deviceManageThread.wait();
    delete deviceManager;
    delete ui;
    delete tc;
    delete ts;
    delete ta;
}

void MainWidget::retranslateUi()
{
    setWindowTitle(tr("vop"));
} // retranslateUi

void MainWidget::initialize()
{
    deviceManager->moveToThread(&deviceManageThread);
    connect(this ,SIGNAL(signals_copy()) ,deviceManager ,SLOT(slots_copy()));
    connect(this ,SIGNAL(signals_device_status()) ,deviceManager ,SLOT(slots_device_status()));
    connect(deviceManager ,SIGNAL(signals_device_status(int)) ,this ,SLOT(slots_device_status(int)));
    deviceManageThread.start();

    connect(&timer ,SIGNAL(timeout()) ,this ,SIGNAL(signals_device_status()));
//    timer.setInterval(1000);
    timer.start(1000);
    //    ps = new QProcess(this);
}

void MainWidget::initializeUi()
{
    on_refresh_clicked();
    //copy initialize
    initializeTabCopy();
    //setting initialize
    ts->setupUi(ui->tab_4);
    //about initialize
    initializeTabAbout();
 }

void MainWidget::createActions()
{
    //    action_refresh = new QAction(this);
    //    connect(action_refresh ,SIGNAL(triggered()) ,this ,SLOT(on_refresh_clicked()));
}
void MainWidget::slots_device_status(int _status)
{
    status = _status;
    updateUi();
}

#include <QMouseEvent>
bool MainWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(qobject_cast<QComboBox*>(obj))
    {
        if(event->type() == QEvent::Wheel)
            return true;
        return QWidget::eventFilter(obj, event);
    }

    if (obj == ta->label && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if(Qt::LeftButton == mouseEvent->button()
                && mouseEvent->x() > 130 && mouseEvent->y() > 280
                && mouseEvent->x() < 180 && mouseEvent->y() < 330
                )
        {
//            action_about_update->trigger();
            slots_about_update();
            qDebug() << "pos:" << mouseEvent->pos();
        }
        return true;
    } else {
        // standard event processing
        return QWidget::eventFilter(obj, event);
    }
}

void MainWidget::updateUi()
{
    updateCopy();
    const char* device_uri = deviceManager->getCurrentDeviceURI();
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(parent());
    if(mainWindow)
    {
        if(device_uri)
        {
            QString title;
            title = ui->comboBox_deviceList->currentText() + " - " + device_uri;
               mainWindow->setWindowTitle(title) ;
        }
        else
        {
               mainWindow->setWindowTitle("vop");
        }
    }
    update();
}

void MainWidget::on_refresh_clicked()
{
    ui->comboBox_deviceList->clear();
    QStringList printerNames;
    int selected_printer = deviceManager->getDeviceList(printerNames);
    if(-1 != selected_printer)//has printer
    {
        ui->comboBox_deviceList->insertItems(0 ,printerNames);
        ui->comboBox_deviceList->setCurrentIndex(selected_printer);
    }
}

void MainWidget::on_comboBox_deviceList_activated(int index)
{
    deviceManager->selectDevice(index);
    updateUi();
}

///////////////////////////////////////////////////////////tab about/////////////////////////////////////////////////////////
void MainWidget::initializeTabAbout()
{
    ta->setupUi(ui->tab_5);
//    ta->label->setMask(QPixmap(QString::fromUtf8(":/images/about.png")).mask());

//    action_about_update = new QAction(this);
//    connect(action_about_update ,SIGNAL(triggered()) ,this ,SLOT(slots_about_update()));

    ta->label->installEventFilter(this);
}

#include <QDesktopServices>
//#include<QUrl>
void MainWidget::slots_about_update()
{
//    ps->start(QLatin1String(sys_open) + "http://www.lenovo.com");
    QDesktopServices::openUrl(QUrl("http://www.lenovo.com"));
}

/////////////////////////////////////////////////////tab copy/////////////////////////////////////////////////////////////
static const char* output_size_list[] =
{
    "Letter (8.5 * 11)" ,
    "A4 (210 * 297mm)",
    "A5 (148 * 210mm)",
    "A6 (105 * 148mm)",
    "B5 (182 * 257mm)",
    "B6 (128 * 182mm)",
    "Executive (7.25 * 105\")",
    "16K (185 * 260mm)"
};

static const int document_size[] =
{0 ,1 ,2 ,4 ,6};//Letter A4 A5 B5 Executive

static const int output_size[][2] =
{
  {216, 279},//Letter
    {210,297},//A4
    {148,210},//A5
    {105,148},//A6
    {182,257},//B5
    {128 ,182},//B6
    {184 ,267},//Executive
    {185 ,260}//16K
};

#define _GetSizeScaling(ds ,os ,index)  ((output_size[os][index] - 8.2) / (output_size[document_size[ds]][index] - 8.2))
#define GetSizeScaling(ds ,os ,scaling) \
{ \
    double  scaling_width = _GetSizeScaling(ds ,os ,0);  \
    double scaling_height = _GetSizeScaling(ds ,os ,1); \
    scaling = 100 * (scaling_width < scaling_height ? scaling_width :scaling_height); \
}

void MainWidget::initializeTabCopy()
{
    tc->setupUi(ui->tab_3);
    int i;
    for(i = 0 ;i < sizeof(output_size_list) / sizeof(output_size_list[0]) ;i++)
        stringlist_output_size << output_size_list[i];
    connect(tc->scaling_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_minus_plus()));
    connect(tc->scaling_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_minus_plus()));
    connect(tc->copies_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_minus_plus()));
    connect(tc->copies_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_minus_plus()));
    connect(tc->density_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_minus_plus()));
    connect(tc->density_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_minus_plus()));
    connect(tc->photo ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_copy_scaningMode()));
//    connect(tc->text ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_scaningMode()));
    connect(tc->IDCardCopy ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_scaningMode()));
    connect(tc->combo_documentType ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->combo_documentSize ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->combo_outputSize ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->combo_nIn1Copy ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->combo_dpi ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->btn_default ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_default()));
    connect(tc->copy ,SIGNAL(clicked()) ,this ,SIGNAL(signals_copy()));

//    action_copy_default = new QAction(this);
//    connect(action_copy_default ,SIGNAL(triggered()) ,this ,SLOT(slots_copy_default()));
//    connect(tc->btn_default ,SIGNAL(clicked()) ,action_copy_default ,SLOT(trigger()));

    ui->comboBox_deviceList->installEventFilter(this);
    tc->combo_documentType->installEventFilter(this);
    tc->combo_documentSize->installEventFilter(this);
    tc->combo_outputSize->installEventFilter(this);
    tc->combo_nIn1Copy->installEventFilter(this);
    tc->combo_dpi->installEventFilter(this);
}

void MainWidget::slots_copy_combo(int value)
{
    QComboBox* qb = qobject_cast<QComboBox*>(sender( ));
    if(!qb)
        return;
    copycmdset copyPara = deviceManager->getCopyParameter();
    copycmdset* pCopyPara = &copyPara;
    if(qb == tc->combo_documentType)              pCopyPara->mediaType = value;
    else if(qb == tc->combo_dpi)                           pCopyPara->dpi = value;
    else if(qb == tc->combo_documentSize)
    {
        pCopyPara->orgSize = value;
        if(!pCopyPara->nUp)
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }
    else if(qb == tc->combo_nIn1Copy)
    {
        pCopyPara->nUp = value;
        QStringList sl(stringlist_output_size);
        if(value)
        {
            pCopyPara->scale = 100;
            if(1 == value)//2 in 1 copy
            {
                //tc->combo_outputSize hide a6 b6
                sl.removeAt(5);
                sl.removeAt(3);
            }else{
                //tc->combo_outputSize only letter a4
                sl.removeLast();
                sl.removeLast();
                sl.removeLast();
                sl.removeLast();
                sl.removeLast();
                sl.removeLast();
            }
        }else{
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
        }

        tc->combo_outputSize->clear();
        tc->combo_outputSize->insertItems(0 ,sl);
    }
    else if(qb == tc->combo_outputSize)
    {
        if(1 == pCopyPara->nUp)
        {
            if(value > 3)                value += 2;
            else if(value > 2)        value ++;
        }
        tc->combo_nIn1Copy->clear();
        if(3 == value || 5 == value)
        {
            tc->combo_nIn1Copy->addItem("1");
        }else if(value > 1){
            tc->combo_nIn1Copy->addItem("1");
            tc->combo_nIn1Copy->addItem("2");
        }else{
            tc->combo_nIn1Copy->addItem("1");
            tc->combo_nIn1Copy->addItem("2");
            tc->combo_nIn1Copy->addItem("4");
            tc->combo_nIn1Copy->addItem("9");
        }

        pCopyPara->paperSize = value;
        if(!pCopyPara->nUp)
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
        qDebug()<<"outsize"<<stringlist_output_size.at(value);
    }
    deviceManager->setCopyParameter(pCopyPara);
    updateUi();
}

void MainWidget::slots_copy_scaningMode()
{
    QAbstractButton* ab = qobject_cast<QAbstractButton*>(sender( ));
    if(!ab)
        return;
    copycmdset copyPara = deviceManager->getCopyParameter();
    copycmdset* pCopyPara = &copyPara;
    if(2 == pCopyPara->scanMode)
    {
        if(ab == tc->IDCardCopy)
        {
            if(tc->photo->isChecked())
                pCopyPara->scanMode = 0;
            else
                pCopyPara->scanMode = 1;
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
        }
    }else{
        if(ab == tc->photo)
        {
            pCopyPara->scanMode = !tc->photo->isChecked();
        }
//        else if(ab == tc->text)
//        {
//            if(1 == pCopyPara->scanMode)
//                return;
//            pCopyPara->scanMode = 1;
//        }
        else if(ab == tc->IDCardCopy)
        {
            pCopyPara->scanMode = 2;
            pCopyPara->dpi = 1;
            pCopyPara->scale = 100;
        }
    }
    deviceManager->setCopyParameter(pCopyPara);
    updateUi();
}

void MainWidget::slots_copy_minus_plus()
{
    QPushButton* pb = qobject_cast<QPushButton*>(sender( ));
    if(!pb)
        return;
    copycmdset copyPara = deviceManager->getCopyParameter();
    copycmdset* pCopyPara = &copyPara;
    if(pb == tc->scaling_minus){
        pCopyPara->scale --;
    }else if(pb == tc->scaling_plus){
        pCopyPara->scale ++;
    }else if(pb == tc->density_minus){
        pCopyPara->Density --;
    }else if(pb == tc->density_plus){
        pCopyPara->Density ++;
    }else if(pb == tc->copies_minus){
        pCopyPara->copyNum --;
    }else if(pb == tc->copies_plus){
        pCopyPara->copyNum ++;
    }
    deviceManager->setCopyParameter(pCopyPara);
    updateUi();
}

#define SetWhite(widget) widget->setStyleSheet("background-color:white")
#define SetGray(widget) widget->setStyleSheet("background-color:gray")
#define SetIDCardCopy(widget) widget->setStyleSheet(":enabled:!pressed{background-color:#faaf40}")
#define SetCopy(widget) widget->setStyleSheet(":enabled:!pressed{background-color:#71c2cd}")

void MainWidget::updateCopy()
{
    copycmdset copyPara = deviceManager->getCopyParameter();
    copycmdset* pCopyPara = &copyPara;
    tc->scaling->setText(QString("%1%").arg(pCopyPara->scale));
    tc->copies->setText(QString("%1").arg(pCopyPara->copyNum));
//    SetGray(tc->density1);
    switch(pCopyPara->Density)
    {
    case 1:SetWhite(tc->density2);SetWhite(tc->density3);SetWhite(tc->density4);SetWhite(tc->density5);break;
    case 2:SetGray(tc->density2);SetWhite(tc->density3);SetWhite(tc->density4);SetWhite(tc->density5);break;
    case 3:SetGray(tc->density2);SetGray(tc->density3);SetWhite(tc->density4);SetWhite(tc->density5);break;
    case 4:SetGray(tc->density2);SetGray(tc->density3);SetGray(tc->density4);SetWhite(tc->density5);break;
    default:SetGray(tc->density2);SetGray(tc->density3);SetGray(tc->density4);SetGray(tc->density5);break;
    }
    switch(pCopyPara->scanMode)
    {
    case 0:        tc->photo->setChecked(true);     break;//touge again
    case 1:        tc->text->setChecked(true);         break;
    default: break;
    }

    int value = pCopyPara->paperSize;
    if(1 == pCopyPara->nUp)
    {
        if(value > 4)                value -= 2;
        else if(value > 2)        value --;
    }
    tc->combo_outputSize->setCurrentIndex(value);
    tc->combo_documentType->setCurrentIndex(pCopyPara->mediaType);
    tc->combo_documentSize->setCurrentIndex(pCopyPara->orgSize);
    tc->combo_nIn1Copy->setCurrentIndex(pCopyPara->nUp);
    tc->combo_dpi->setCurrentIndex(pCopyPara->dpi);

    //check valid
    if(pCopyPara->scale >= 400) {pCopyPara->scale = 400; tc->scaling_plus->setEnabled(false);}
    else{tc->scaling_plus->setEnabled(true);}
    if(pCopyPara->scale <= 25) {pCopyPara->scale = 25; tc->scaling_minus->setEnabled(false);}
    else{tc->scaling_minus->setEnabled(true);}

    if(pCopyPara->copyNum >= 100) {pCopyPara->copyNum = 100; tc->copies_plus->setEnabled(false);}
    else{tc->copies_plus->setEnabled(true);}
    if(pCopyPara->copyNum <= 1) {pCopyPara->copyNum = 1; tc->copies_minus->setEnabled(false);}
    else{tc->copies_minus->setEnabled(true);}

    if(pCopyPara->Density >= 5) {pCopyPara->Density = 5; tc->density_plus->setEnabled(false);}
    else{tc->density_plus->setEnabled(true);}
    if(pCopyPara->Density <= 1) {pCopyPara->Density = 1; tc->density_minus->setEnabled(false);}
    else{tc->density_minus->setEnabled(true);}

    if(2 == pCopyPara->scanMode)
    {
        tc->scaling_plus->setEnabled(false);
        tc->scaling_minus->setEnabled(false);
        tc->combo_documentSize->setEnabled(false);
        tc->combo_nIn1Copy->setEnabled(false);
        tc->combo_dpi->setEnabled(false);
        SetIDCardCopy(tc->copy);
    }
    else
    {
        if(pCopyPara->scale < 400) {tc->scaling_plus->setEnabled(true);}
        if(pCopyPara->scale > 25) {tc->scaling_minus->setEnabled(true);}
        tc->combo_documentSize->setEnabled(true);
        tc->combo_nIn1Copy->setEnabled(true);
        tc->combo_dpi->setEnabled(true);
        SetCopy(tc->copy);
    }

    if(pCopyPara->nUp)
    {
        tc->IDCardCopy->setEnabled(false);
        tc->scaling_plus->setEnabled(false);
        tc->scaling_minus->setEnabled(false);
    }
    else
    {
        tc->IDCardCopy->setEnabled(true);
        if(pCopyPara->scale < 400) {tc->scaling_plus->setEnabled(true);}
        if(pCopyPara->scale > 25) {tc->scaling_minus->setEnabled(true);}
    }
    if(!status)//device status ready
        tc->copy->setEnabled(true);
    else
        tc->copy->setEnabled(false);
}

void MainWidget::slots_copy_default()
{
    deviceManager->setDefaultCopyParameter();
    updateUi();
}
