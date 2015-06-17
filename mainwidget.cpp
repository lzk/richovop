/////////////////////////////////////////
/// File:mainwidget.cpp
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "ui_copy.h"
#include "ui_setting.h"
#include "ui_about.h"

#include <QAction>
#include <QDebug>
#include <QMainWindow>
#include <QProgressDialog>

#include "app/devicemanager.h"
#include "app/deviceapp.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    tc(new Ui::TabCopy),
    ts(new Ui::TabSetting),
    ta(new Ui::TabAbout),
    device_status(0)
{
    device = new DeviceApp(this);
    ui->setupUi(this);
    createActions();
    initializeUi();
    retranslateUi();

    initialize();
}

MainWidget::~MainWidget()
{
    delete ui;
    delete tc;
    delete ts;
    delete ta;
//    delete progressDialog;
}

QMessageBox::StandardButton MainWidget::showMessageBox(const QString &text,
          QMessageBox::StandardButtons buttons ,
         QMessageBox::StandardButton defaultButton,
         const QString &title )
{
    MessageBox msgBox;
    msgBox.setText(title);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setInformativeText(text);
    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(defaultButton);
    msgBox.setWindowFlags(Qt::FramelessWindowHint);
    return (QMessageBox::StandardButton)msgBox.exec();
}

void MainWidget::retranslateUi()
{
} // retranslateUi

void MainWidget::initialize()
{
    connect(&timer ,SIGNAL(timeout()) ,this ,SLOT(slots_timeout()));
//    timer.setInterval(1000);
    timer.start(1000);

    on_refresh_clicked();
    updateUi();

//    progressDialog = new QProgressDialog(this ,Qt::SplashScreen);
    progressDialog = new QProgressDialog(this);
    progressDialog->setCancelButton(NULL);
    progressDialog->setLabel(new QLabel(tr("Get Printer Information.")));
    progressDialog->setWindowFlags(Qt::FramelessWindowHint);

}

void MainWidget::slots_progressBar(int value)
{
    progressDialog->setValue(value);
}

void MainWidget::slots_timeout()
{
    static int count = 0;
    switch(device->cmd_status)
    {
    case DeviceManager::CMD_STATUS_COMPLETE://jobs complete,no job
        if(0 == count % 10)
            emit_cmd(DeviceManager::CMD_DEVICE_status);
        break;

    //others cmd not complete
    case DeviceManager::CMD_WIFI_getAplist:
    case DeviceManager::CMD_WIFI_get:
    case DeviceManager::CMD_DEVICE_status:
    case DeviceManager::CMD_COPY:
    case DeviceManager::CMD_WIFI_apply:
    default://
        break;
    }
    count ++;
    if(count >= 100)
        count = 0;
}

void MainWidget::slots_cmd()
{
    QObject* sd = sender();
    if(sd == tc->copy)    {
        emit_cmd(DeviceManager::CMD_COPY);
    }else if(sd == ts->btn_apply_ws){
        slots_wifi_applyDo();
    }else if(sd == ts->btn_refresh){
        emit_cmd(DeviceManager::CMD_WIFI_getAplist);
    }else if(sd == ts->btn_apply_mp){
        slots_passwd_setDo();
    }
}
void MainWidget::emit_cmd(int cmd)
{
    if(!device->emit_cmd(cmd)){
        showMessageBox(tr("The machine is busy, please try later..."));
    }
}

void MainWidget::slots_cmd_result(int cmd ,int err)
{
    qDebug()<<"err:"<<tr(VopProtocol::getErrString(err));
    switch(err){//handle err message box
    case ERR_communication ://communication err
        if(DeviceManager::CMD_DEVICE_status != cmd)
            showMessageBox(tr("Failed to acquire the information."));
        break;
    case ERR_Password_incorrect :
        showMessageBox(tr("Authentication error, please enter the password again."));
        break;
    case ERR_CMD_invalid :
    case ERR_Parameter_invalid :
    case ERR_Do_not_support :
    case ERR_Printer_busy :
    case ERR_Printer_error :
    case ERR_Set_parameter_error :
    case ERR_Get_parameter_error :
    case ERR_Printer_is_Sleeping:
    case ERR_Printer_is_in_error:
    case ERR_Scanner_operation_NG :
    case ERR_ACK :
    default:
        break;
    }
    switch(cmd)
    {
    case DeviceManager::CMD_DEVICE_status:
        device_status = (err ==  ERR_ACK);
//        updateCopy();//disable copy or enable
        tc->copy->setEnabled(device_status);
        break;
    case DeviceManager::CMD_WIFI_get:
        if(!err){//no err,ACK
            slots_wifi_get();
        }
        emit ts->checkBox->toggled(ts->checkBox->isChecked());//update setting enable or disbale
        break;
    case DeviceManager::CMD_PASSWD_confirmForApply:
        if(!err){//no err,ACK
            passwd_checked = true;
            slots_wifi_applyDone();
        }else if(ERR_Password_incorrect == err){//password incorrect
            passwd_checked = false;
            slots_wifi_applyDo();
        }
        break;
    case DeviceManager::CMD_WIFI_apply:
        if(!err){
            //clear passwd
//            wifi_ms_password.clear();
//            wifi_sw_password.clear();
//            ts->le_passphrase->clear();
//            ts->le_wepkey->clear();
            wifi_update();
        }
        break;
    case DeviceManager::CMD_PASSWD_confirmForSetPasswd:
        if(!err){//no err,ACK
            slots_passwd_setDone();
        }else if(ERR_Password_incorrect == err){//password incorrect
            passwd_checked = false;
            slots_passwd_setDo();
        }
        break;
    case DeviceManager::CMD_PASSWD_set:
        if(!err){
            ts->le_confirmPassword->clear();
            ts->le_newPassword->clear();
        }
        break;
    case DeviceManager::CMD_WIFI_getAplist:
        if(!err){//no err,ACK
//        if(1){//no err,ACK
            slots_wifi_getAplist();
        }
        break;
    default:
        break;
    }
    device->cmd_status = DeviceManager::CMD_STATUS_COMPLETE;
}
void MainWidget::initializeUi()
{
    initializeTabCopy();
    initializeTabSetting();
    initializeTabAbout();
 }

void MainWidget::createActions()
{
    //    action_refresh = new QAction(this);
    //    connect(action_refresh ,SIGNAL(triggered()) ,this ,SLOT(on_refresh_clicked()));
}

#include <QMouseEvent>
bool MainWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(qobject_cast<QComboBox*>(obj))    {
        if(event->type() == QEvent::Wheel)
            return true;
    }    else    if(obj == ts->pageWidget && event->type() == QEvent::Show)    {//wifi setup show
        emit_cmd(DeviceManager::CMD_WIFI_get);
    }    else        if(obj == ui->tab_4 && event->type() == QEvent::Hide)        {//tab setting hide
            passwd_checked = false;
    }    else    if (obj == ta->label && event->type() == QEvent::MouseButtonPress) {//tab about click
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if(Qt::LeftButton == mouseEvent->button()
                && mouseEvent->x() > 130 && mouseEvent->y() > 280
                && mouseEvent->x() < 180 && mouseEvent->y() < 330
                )        {
//            action_about_update->trigger();
            slots_about_update();
//            qDebug() << "pos:" << mouseEvent->pos();
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void MainWidget::updateUi()
{
    DeviceManager* deviceManager = device->deviceManager;
    QString device_uri = deviceManager->getCurrentDeviceURI();
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(parent());
    if(mainWindow){
        if(!device_uri.isEmpty()){
            QString title;
            title = ui->comboBox_deviceList->currentText() + " - " + device_uri;
               mainWindow->setWindowTitle(title) ;
        }else{
               mainWindow->setWindowTitle(tr("Ricoh VOP"));
        }
    }
//    update();
}

void MainWidget::on_refresh_clicked()
{
    ui->comboBox_deviceList->clear();
    QStringList printerNames;
    DeviceManager* deviceManager = device->deviceManager;
    int selected_printer = deviceManager->getDeviceList(printerNames);
    if(-1 != selected_printer)//has printer
    {
        ui->comboBox_deviceList->insertItems(0 ,printerNames);
        ui->comboBox_deviceList->setCurrentIndex(selected_printer);
        on_comboBox_deviceList_activated(selected_printer);
    }
}

void MainWidget::on_comboBox_deviceList_activated(int index)
{
    DeviceManager* deviceManager = device->deviceManager;
    deviceManager->selectDevice(index);    
    updateUi();
    ui->tabWidget->setCurrentWidget(ui->tab_5);
    emit_cmd(DeviceManager::CMD_DEVICE_status);
}

///////////////////////////////////////////////////////////tab about/////////////////////////////////////////////////////////
void MainWidget::initializeTabAbout()
{
    ta->setupUi(ui->tab_5);
//    action_about_update = new QAction(this);
//    connect(action_about_update ,SIGNAL(triggered()) ,this ,SLOT(slots_about_update()));
    ta->label->installEventFilter(this);
}

#include <QDesktopServices>
//#include<QUrl>
void MainWidget::slots_about_update()
{
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

    updateCopy();
    connect(tc->btn_default ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(tc->scaling_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(tc->scaling_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(tc->copies_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(tc->copies_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(tc->density_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(tc->density_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
//    connect(tc->text ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_copy_pushbutton()));
    connect(tc->IDCardCopy ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(tc->photo ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_copy_radio(bool)));
    connect(tc->combo_documentType ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->combo_documentSize ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->combo_outputSize ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->combo_nIn1Copy ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->combo_dpi ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(tc->copy ,SIGNAL(clicked()) ,this ,SLOT(slots_cmd()));

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

#define SetWhite(widget) widget->setStyleSheet("background-color:white")
#define SetGray(widget) widget->setStyleSheet("background-color:gray")
#define SetIDCardCopy(widget) widget->setStyleSheet(":enabled:!pressed{background-color:#faaf40}")
#define SetCopy(widget) widget->setStyleSheet(":enabled:!pressed{background-color:#71c2cd}")

void MainWidget::updateCopy()
{
    DeviceManager* deviceManager = device->deviceManager;
    copycmdset copyPara = deviceManager->copy_get_para();
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

    QStringList sl(stringlist_output_size);
    if(1 == pCopyPara->nUp){//2in1 hide a6 b6
        sl.removeAt(5);
        sl.removeAt(3);
        if(3 == pCopyPara->paperSize || 5 == pCopyPara->paperSize){
            pCopyPara->paperSize = 0;
        }
    }else if(pCopyPara->nUp) {// 4in1 and 9in1 only letter a4
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        if(pCopyPara->paperSize > 1)
            pCopyPara->paperSize = 0;
    }
    tc->combo_outputSize->clear();
    tc->combo_outputSize->insertItems(0 ,sl);

    int value = pCopyPara->paperSize;
    tc->combo_nIn1Copy->clear();
    if(3 == value || 5 == value){
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

    if(pCopyPara->copyNum >= 99) {pCopyPara->copyNum = 99; tc->copies_plus->setEnabled(false);}
    else{tc->copies_plus->setEnabled(true);}
    if(pCopyPara->copyNum <= 1) {pCopyPara->copyNum = 1; tc->copies_minus->setEnabled(false);}
    else{tc->copies_minus->setEnabled(true);}

    if(pCopyPara->Density >= 5) {pCopyPara->Density = 5; tc->density_plus->setEnabled(false);}
    else{tc->density_plus->setEnabled(true);}
    if(pCopyPara->Density <= 1) {pCopyPara->Density = 1; tc->density_minus->setEnabled(false);}
    else{tc->density_minus->setEnabled(true);}

    if(2 == pCopyPara->scanMode)
    {
//        tc->scaling_plus->setEnabled(false);
//        tc->scaling_minus->setEnabled(false);
        tc->bg_scaling->setEnabled(false);
        tc->combo_documentSize->setEnabled(false);
        tc->combo_nIn1Copy->setEnabled(false);
        tc->combo_dpi->setEnabled(false);
        SetIDCardCopy(tc->copy);
    }
    else
    {
        tc->bg_scaling->setEnabled(true);
//        if(pCopyPara->scale < 400) {tc->scaling_plus->setEnabled(true);}
//        if(pCopyPara->scale > 25) {tc->scaling_minus->setEnabled(true);}
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
    tc->copy->setEnabled(device_status);
}

void MainWidget::slots_copy_combo(int value)
{
    QObject* sd = sender();
    DeviceManager* deviceManager = device->deviceManager;
    copycmdset copyPara = deviceManager->copy_get_para();
    copycmdset* pCopyPara = &copyPara;
    if(sd == tc->combo_documentType)
        pCopyPara->mediaType = value;
    else if(sd == tc->combo_dpi)
        pCopyPara->dpi = value;
    else if(sd == tc->combo_documentSize) {//disable when scanMode ==2
        pCopyPara->orgSize = value;
        if(!pCopyPara->nUp)// && 2 > pCopyPara->scanMode)//always true
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }else if(sd == tc->combo_nIn1Copy){//disable when scanMode ==2
        pCopyPara->nUp = value;
        if(value)        {
            pCopyPara->scale = 100;
        }else{
//            if(2 > pCopyPara->scanMode)//always true
                GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
        }
    }    else if(sd == tc->combo_outputSize)    {
        if(1 == pCopyPara->nUp)        {
            if(value > 3)                value += 2;
            else if(value > 2)        value ++;
        }
        pCopyPara->paperSize = value;
        if(!pCopyPara->nUp && 2 > pCopyPara->scanMode)
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }
    deviceManager->copy_set_para(pCopyPara);
    updateCopy();
}

void MainWidget::slots_copy_pushbutton()
{
    DeviceManager* deviceManager = device->deviceManager;
    copycmdset copyPara = deviceManager->copy_get_para();
    copycmdset* pCopyPara = &copyPara;
    QObject* sd = sender();
    if(sd == tc->IDCardCopy){//button IDCardCopy click
        if(2 == pCopyPara->scanMode)    {//ID Card mode
            if(tc->photo->isChecked())
                pCopyPara->scanMode = 0;
            else
                pCopyPara->scanMode = 1;
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
        }else{
            pCopyPara->scanMode = 2;
            pCopyPara->dpi = 1;//600 * 600
            pCopyPara->scale = 100;
        }
    }else if(sd == tc->btn_default){
        deviceManager->copy_set_defaultPara();
        updateCopy();
        return;
    }else if(sd == tc->scaling_minus){
        pCopyPara->scale --;
    }else if(sd == tc->scaling_plus){
        pCopyPara->scale ++;
    }else if(sd == tc->density_minus){
        pCopyPara->Density --;
    }else if(sd == tc->density_plus){
        pCopyPara->Density ++;
    }else if(sd == tc->copies_minus){
        pCopyPara->copyNum --;
    }else if(sd == tc->copies_plus){
        pCopyPara->copyNum ++;
    }
    deviceManager->copy_set_para(pCopyPara);
    updateCopy();
}

void MainWidget::slots_copy_radio(bool checked)
{
    QObject* sd = sender();
    if(sd == tc->photo){
        DeviceManager* deviceManager = device->deviceManager;
        copycmdset copyPara = deviceManager->copy_get_para();
        copycmdset* pCopyPara = &copyPara;
        if(2 != pCopyPara->scanMode){//not ID Card mode
            pCopyPara->scanMode = !checked;
            deviceManager->copy_set_para(pCopyPara);
            updateCopy();
        }
    }
}

//////////////////////////tab setting///////////////////
void MainWidget::initializeTabSetting()
{
    ts->setupUi(ui->tab_4);

    ts->listWidget->setCurrentRow(0);
    ts->stackedWidget->setCurrentIndex(0);
    ts->radioButton_searchWifi->setChecked(true);
    ts->searchWifiWidget->show();
    ts->manualSetupWidget->hide();

    wifi_encryptionType = 2;
    wifi_ms_wepIndex = 0;
    wifi_sw_wepIndex = 0;
    wifi_sw_encryptionType[0] = 2;
    passwd_checked = false;
    wifi_update();

    QRegExp regexp("^[\\x0020-\\x007e]{1,32}$");
    QValidator *validator = new QRegExpValidator(regexp, this);
    ts->le_ssid->setValidator(validator);

    regexp.setPattern("^[0-9a-zA-Z]{1,32}$");
    QValidator* validator2 =  new QRegExpValidator(regexp, this);
    ts->le_newPassword->setValidator(validator2);
    ts->le_confirmPassword->setValidator(validator2);

    connect(ts->radioButton_searchWifi ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_radiobutton(bool)));
    connect(ts->le_ssid,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ts->le_wepkey,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ts->le_passphrase,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ts->cb_encryptionType,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ts->cb_keyIndex,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ts->cb_ssid,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ts->btn_apply_ws ,SIGNAL(clicked()) ,this ,SLOT(slots_cmd()));
    connect(ts->btn_refresh ,SIGNAL(clicked()) ,this ,SLOT(slots_cmd()));
    connect(ts->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_checkbox(bool)));
    connect(ts->btn_apply_mp ,SIGNAL(clicked()) ,this ,SLOT(slots_cmd()));

    ts->pageWidget->installEventFilter(this);
    ts->cb_encryptionType->installEventFilter(this);
    ts->cb_keyIndex->installEventFilter(this);
    ts->cb_ssid->installEventFilter(this);
    ui->tab_4->installEventFilter(this);
}

void MainWidget::wifi_update_encryptionType()
{
    switch(wifi_encryptionType)
    {
    case 1://wep
        ts->passphraseWidget->hide();
        ts->wepWidget->show();
        break;
    case 2://wpa2-psk-aes
    case 3://mixed mode psk
        ts->passphraseWidget->show();
        ts->wepWidget->hide();
        break;
    default:
        ts->passphraseWidget->hide();
        ts->wepWidget->hide();
        break;
    }
}

void MainWidget::slots_wifi_radiobutton(bool checked)
{
    QObject* sd = sender();
    if(sd == ts->radioButton_searchWifi){
        if(checked){//show wifi setup
            ts->searchWifiWidget->show();
            ts->manualSetupWidget->hide();
            //search wifi password
            ts->le_wepkey->setText(wifi_sw_password);
            ts->le_passphrase->setText(wifi_sw_password);
            ts->cb_keyIndex->setCurrentIndex(wifi_sw_wepIndex);
        }else{//show manual setup
            ts->searchWifiWidget->hide();
            ts->manualSetupWidget->show();
            //manual setup password
            ts->le_wepkey->setText(wifi_ms_password);
            ts->le_passphrase->setText(wifi_ms_password);
            ts->cb_keyIndex->setCurrentIndex(wifi_ms_wepIndex);
        }
        //clear password when radio button emit
//        ts->le_wepkey->clear();
//        ts->le_passphrase->clear();
        wifi_update();
    }
}

void MainWidget::wifi_update()
{
    wifi_update_Data();
    wifi_update_encryptionType();
    ts->btn_apply_ws->setEnabled(wifi_validate_ssidPassword());
}

void MainWidget::wifi_update_Data()
{
    if(ts->radioButton_searchWifi->isChecked()){
        wifi_ssid = ts->cb_ssid->currentText();
        int index = ts->cb_ssid->currentIndex();
        wifi_encryptionType = wifi_sw_encryptionType[index < 0 ?0 :index];
        wifi_wepIndex = wifi_sw_wepIndex;
    }else{
        wifi_encryptionType = ts->cb_encryptionType->currentIndex();
        wifi_ssid = ts->le_ssid->text();
        wifi_wepIndex = wifi_ms_wepIndex;
    }
    if(1 == wifi_encryptionType){
        wifi_password = ts->le_wepkey->text();
    }else{
        wifi_password = ts->le_passphrase->text();//not use when encryptionType=0
    }
}

void MainWidget::slots_wifi_textChanged(const QString &arg1)
{
    QObject* sd = sender();
    if(sd == ts->le_passphrase){
        if(ts->radioButton_searchWifi->isChecked()){
            wifi_sw_password = arg1;
        }else{
            wifi_ms_password = arg1;
        }
        wifi_update();
    }else if(sd == ts->le_wepkey){
        if(ts->radioButton_searchWifi->isChecked()){
            wifi_sw_password = arg1;
        }else{
            wifi_ms_password = arg1;
        }
        wifi_update();
    }else if(sd == ts->le_ssid){
        wifi_update();
    }else  if(sd == ts->cb_encryptionType){
//        wifi_ms_password.clear();
//        ts->le_passphrase->clear();
//        ts->le_wepkey->clear();
        wifi_update();
    }else if(sd == ts->cb_ssid){
        wifi_sw_password.clear();
        ts->le_passphrase->clear();
        ts->le_wepkey->clear();
        wifi_update();
    }else if(sd == ts->cb_keyIndex){
        if(ts->radioButton_searchWifi->isChecked()){
            wifi_sw_wepIndex = ts->cb_keyIndex->currentIndex();
        }else{
            wifi_ms_wepIndex = ts->cb_keyIndex->currentIndex();
        }
    }
}

bool MainWidget::wifi_validate_ssidPassword()
{
    bool validPattern = false;
    if(wifi_encryptionType){
        QRegExp regexp;
        if(1 == wifi_encryptionType)
        {
            regexp.setPattern("^(?:.{5,5}|.{13,13}|[0-9a-fA-F]{10,10}|[0-9a-fA-F]{26,26})$");
        }else{
            regexp.setPattern("^(?:.{8,63}|[0-9a-fA-F]{64,64})$");
        }
        if(!wifi_ssid.isEmpty() && !wifi_password.isEmpty() && -1 != regexp.indexIn(wifi_password))
            validPattern = true;
    }else{
        if(!wifi_ssid.isEmpty())
             validPattern = true;
    }
    return validPattern;
}

void MainWidget::slots_wifi_checkbox(bool checked)
{
    if(checked){
        ts->frame->setEnabled(true);
        ts->btn_apply_ws->setEnabled(wifi_validate_ssidPassword());
    }else{//diable setting
        ts->frame->setEnabled(false);
        ts->btn_apply_ws->setEnabled(true);
    }
}

#include <QInputDialog>
void MainWidget::slots_wifi_applyDo()
{
    if(passwd_checked){
        emit_cmd(DeviceManager::CMD_PASSWD_confirmForApply);
    }else{
        bool ok;
        passwd = QInputDialog::getText(this ,tr("Login") ,tr("Password") ,QLineEdit::Password ,QString() ,&ok);
        if (ok && !passwd.isEmpty()){
            DeviceManager* deviceManager = device->deviceManager;
            deviceManager->passwd_set(passwd.toLatin1());
            emit_cmd(DeviceManager::CMD_PASSWD_confirmForApply);
        }
    }
}

void MainWidget::slots_wifi_applyDone()
{
    DeviceManager* deviceManager = device->deviceManager;
    cmdst_wifi_get wifi_para = deviceManager->wifi_get_para();
    //setting data then apply
    deviceManager->wifi_set_password(&wifi_para ,wifi_password.toLatin1());
    deviceManager->wifi_set_ssid(&wifi_para ,wifi_ssid.toLatin1());
    wifi_para.encryption = wifi_encryptionType > 1 ? wifi_encryptionType + 1 :wifi_encryptionType;
    wifi_para.wepKeyId = wifi_wepIndex;
    wifi_para.wifiEnable &= ~1;
    wifi_para.wifiEnable |= ts->checkBox->isChecked() ? 1 : 0;//bit 0
    deviceManager->wifi_set_para(&wifi_para);
    emit_cmd(DeviceManager::CMD_WIFI_apply);
}

void MainWidget::slots_passwd_setDo()
{
    if(ts->le_newPassword->text().isEmpty() || ts->le_confirmPassword->text().isEmpty())
        return;
    if(QString::compare(ts->le_newPassword->text() ,ts->le_confirmPassword->text())){
        QMessageBox::critical(this ,tr("Lenovo Virtual Panel") ,tr("The passwords you entered are different, please try again."));
    }else{
        bool ok;
        passwd = QInputDialog::getText(this ,tr("Login") ,tr("Password") ,QLineEdit::Password ,QString() ,&ok);
        if (ok && !passwd.isEmpty())
        {
            DeviceManager* deviceManager = device->deviceManager;
            deviceManager->passwd_set(passwd.toLatin1());
            emit_cmd(DeviceManager::CMD_PASSWD_confirmForSetPasswd);
        }
    }
}

void MainWidget::slots_passwd_setDone()
{
    DeviceManager* deviceManager = device->deviceManager;
    deviceManager->passwd_set(ts->le_newPassword->text().toLatin1());
    emit_cmd(DeviceManager::CMD_PASSWD_set);    
}

void MainWidget::slots_wifi_get()
{
 //            ts->pageWidget->setEnabled(true);
        DeviceManager* deviceManager = device->deviceManager;
        cmdst_wifi_get wifi_para = deviceManager->wifi_get_para();
        //wifi enable
        ts->checkBox->setChecked(wifi_para.wifiEnable & 1 ?true :false);
        //ssid
        QString ssid(wifi_para.ssid);
        if(ssid.count() > 32)
            machine_wifi_ssid = QString(wifi_para.ssid).left(32);
        else
            machine_wifi_ssid = ssid;
        ts->le_ssid->setText(machine_wifi_ssid);
        ts->cb_ssid->setCurrentText(machine_wifi_ssid);
        //encryption
        int encryption = wifi_para.encryption % 5;
        if(encryption != 2){
            if(encryption == 3 || encryption == 4)
                    encryption --;
            ts->cb_encryptionType->setCurrentIndex(encryption);
        }
        //key index
        ts->cb_keyIndex->setCurrentIndex(wifi_para.wepKeyId % 4);

        wifi_update();
        //get aplist
        emit ts->btn_refresh->clicked();
}

void MainWidget::slots_wifi_getAplist()
{
    DeviceManager* deviceManager = device->deviceManager;
    cmdst_aplist_get aplist = deviceManager->wifi_getAplist();
    //aplist
    ts->cb_ssid->clear();
    int current_ssid = 0;
    for(int i = 0 ;i < NUM_OF_APLIST ;i++){
//                QString ssid((char*)&aplist.aplist[i]);
        QString ssid = QString(aplist.aplist[i].ssid).left(32);
        if(ssid.isEmpty()){
            break;
        }else{
            ts->cb_ssid->addItem( ssid);
            wifi_sw_encryptionType[i] = aplist.aplist[i].encryption;
            if(!ssid.compare(machine_wifi_ssid))
                current_ssid = i;
        }
    }
    ts->cb_ssid->setCurrentIndex(current_ssid);
    //update ui else
    emit ts->cb_ssid->activated(ts->cb_ssid->currentText());
}
