/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "ui_copy.h"
#include "ui_setting.h"
#include "ui_about.h"

#include <QAction>
#include <QMainWindow>
#include <QProgressDialog>
#include <QDesktopWidget>

#include "app/log.h"
#include "app/devicecontrol.h"
#include "app/deviceapp.h"
#include "scalingsettingkeyboard.h"
#include "copiessettingkeyboard.h"
#include "app/devicemanager.h"
MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    tc(new Ui::TabCopy),
    ts(new Ui::TabSetting),
    ta(new Ui::TabAbout),
    device_status(0)
{
    device_manager = new DeviceManager(this);
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
    delete device_manager;
}

QMessageBox::StandardButton MainWidget::messagebox_exec(const QString &text,
          QMessageBox::StandardButtons buttons ,
         QMessageBox::StandardButton defaultButton,
         const QString &title )
{
    MessageBox* mb;
    mb = &msgBox;
//    mb->setParent(this);
    if(mb->isVisible()){
        mb->hide();
    }    
    mb->setText(title);
    mb->setIcon(QMessageBox::Information);
    mb->setInformativeText(text);
    mb->setStandardButtons(buttons);
    mb->setDefaultButton(defaultButton);
    mb->setWindowFlags(Qt::FramelessWindowHint);//
#if 1
    mb->show();//show first before get real size
//    QPoint widget_pos = mapToGlobal(pos());
//    mb->move(widget_pos.x() + (width() - mb->width())/2,
//         widget_pos.y() + (height() - mb->height())/2 - 50);
    mb->move((QApplication::desktop()->width() - mb->width())/2,
          (QApplication::desktop()->height() - mb->height())/2);
#endif
    return (QMessageBox::StandardButton)mb->exec();
}

void MainWidget::messagebox_show(const QString &text,
          QMessageBox::StandardButtons buttons ,
         QMessageBox::StandardButton defaultButton,
         const QString &title )
{
    MessageBox* mb;
    mb = &msgBox_info;
    if(text.compare(mb->informativeText()) && mb->isVisible()){
        mb->hide();
    }
    if(!mb->isVisible()){
        mb->setText(title);
        mb->setIcon(QMessageBox::Information);
        mb->setInformativeText(text);
        mb->setStandardButtons(buttons);
        mb->setDefaultButton(defaultButton);
        mb->setWindowFlags(Qt::FramelessWindowHint);
        mb->adjustSize();
        mb->show();//show first before get real size
#if 1
//        QPoint widget_pos = mapToGlobal(pos());
//        mb->move(widget_pos.x() + (width() - mb->width())/2,
//             widget_pos.y() + (height() - mb->height())/2 - 50);
        mb->move((QApplication::desktop()->width() - mb->width())/2,
              (QApplication::desktop()->height() - mb->height())/2);
#endif
    }
}
void MainWidget::retranslateUi()
{
} // retranslateUi
#include<QProgressBar>
void MainWidget::initialize()
{
    connect(&timer ,SIGNAL(timeout()) ,this ,SLOT(slots_timeout()));
//    timer.setInterval(1000);
    timer.start(1000);

    progressDialog = new QProgressDialog(this);
    progressDialog->setWindowFlags(Qt::SplashScreen);

    progressDialog->setCancelButton(NULL);
    progressDialog->setModal(true);
    progressDialog->setLabel(new QLabel("\n\n" + tr("Get Printer Information.") +"\t\t\t\t\t\n"));

    progressDialog->move((QApplication::desktop()->width() - progressDialog->width())/2,
          (QApplication::desktop()->height() - progressDialog->height())/2);

    msgBox.setWindowTitle(" ");
    msgBox_info.setWindowTitle(" ");

//    on_refresh_clicked();

}

void MainWidget::slots_progressBar(int value)
{
    progressDialog->setValue(value);
    if(!value)
        progressDialog->show();
}

void MainWidget::slots_timeout()
{
    static int count = 0;
    DeviceApp* device_app = device_manager->deviceApp();
    if(device_app){
        switch(device_app->get_cmdStatus())
        {
        case DeviceContrl::CMD_STATUS_COMPLETE://jobs complete,no job
            if(0 == count % 5  && tc->copy->isVisible())
                emit_cmd(DeviceContrl::CMD_DEVICE_status);
            break;

        //others cmd not complete
        case DeviceContrl::CMD_WIFI_getAplist:
        case DeviceContrl::CMD_WIFI_get:
        case DeviceContrl::CMD_DEVICE_status:
        case DeviceContrl::CMD_COPY:
        case DeviceContrl::CMD_WIFI_apply:
        default://
            break;
        }
    }

    count ++;
    if(count >= 100)
        count = 0;
}

void MainWidget::slots_cmd()
{
    QObject* sd = sender();
    if(sd == tc->copy)    {
        tc->copy->setEnabled(false);
        emit_cmd(DeviceContrl::CMD_COPY);
    }else if(sd == ts->btn_apply_ws){
        wifi_passwd_doConfirm(SLOT(slots_passwd_comfirmed()));
    }else if(sd == ts->btn_refresh){
        wifi_getStatusToRefreshAplist();
    }else if(sd == ts->btn_apply_mp){
        passwd_set_doConfirm();
    }
}
void MainWidget::emit_cmd(int cmd)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(device_app && !device_app->emit_cmd(cmd)){
//            messagebox_exec(tr("The machine is busy, please try later..."));
    }
}

void MainWidget::slots_cmd_complete()
{
    disconnect(SIGNAL(signals_cmd_next()));
    DeviceApp* device_app = device_manager->deviceApp();
    device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
}

void MainWidget::cmdResult_getDeviceStatus(int err)
{
    static bool idCardMode = false;
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    if(!err){
        int _status=device_manager->get_deviceStatus();
        switch(_status){
        case PSTATUS_Ready:
        case PSTATUS_PowerSaving:
            device_status = true;
            if(idCardMode){
                idCardMode = false;
                copy_button_IDCardCopy();
                updateCopy();
            }
            break;
        default:
            device_status = false;
            break;
        }
        if(PSTATUS_CopyScanNextPage == _status){
            copycmdset copyPara = device_manager->copy_get_para();
            copycmdset* pCopyPara = &copyPara;
            if(pCopyPara->nUp == 4){//IsIDCardCopyMode(pCopyPara))
                messagebox_show(tr("IDS_MSG_TurnCardOver"));
                idCardMode = true;
            }else
                messagebox_show(tr("IDS_MSG_PlaceNextPage"));
        }
//        else if(PSTATUS_Printing == _status)
//            messagebox_show(tr("IDS_MSG_Printering"));
        else
            messagebox_hide();
        qLog(QString().sprintf("get_deviceStatus correct:%#.2x" ,_status));
//            device_status = device_manager->get_deviceStatus() == PSTATUS_Ready ? true :false;
    }else{
        device_status = false;
    }
//        updateCopy();//disable copy or enable
    tc->copy->setEnabled(device_status);
    device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
}

void MainWidget::cmdResult_passwd_confirmForApply(int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    if(!err){//no err,ACK
        passwd_checked = true;
        wifi_apply();
    }else if(ERR_Password_incorrect == err){//password incorrect
        passwd_checked = false;
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
        wifi_passwd_doConfirm();
    }else{
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
    }
}

void MainWidget::cmdResult_wifi_apply(int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    if(!err){
        //clear passwd
        wifi_ms_password.clear();
        wifi_sw_password.clear();
        ts->le_passphrase->clear();
        ts->le_wepkey->clear();
        wifi_update();
    }
    device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
}

void MainWidget::cmdResult_passwd_confirmForSetPasswd(int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    if(!err){//no err,ACK
        slots_passwd_set();
    }else if(ERR_Password_incorrect == err){//password incorrect
        passwd_checked = false;
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
        passwd_set_doConfirm();
    }else{
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
    }
}

void MainWidget::cmdResult_passwd_set(int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    if(!err){
        ts->le_confirmPassword->clear();
        ts->le_newPassword->clear();
    }
    device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
}

void MainWidget::cmdResult_wifi_getAplist(int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    if(!err){
        result_wifi_getAplist();
    }
    device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
}

int MainWidget::cmdResult_emit_next(int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!err){
        emit signals_cmd_next();
    }else{
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
    }
    return err;
}

void MainWidget::cmdResult_wifi_get(int err)
{
    //for last version
//    DeviceApp* device_app = device_manager->deviceApp();
//    if(!device_app)
//        return;
//    if(!err){
//        emit_cmd(DeviceContrl::CMD_WIFI_getAplist);
//    }else{
//        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
//        wifi_update_checkbox(ts->checkBox->isChecked());
//    }
    if(!cmdResult_emit_next(err)){
        wifi_update_checkbox(ts->checkBox->isChecked());
    }
}

void MainWidget::slots_cmd_result(int cmd ,int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    qLog(QString("cmd return:") + VopProtocol::getErrString(err));
    //handle err message box
    switch(err){
    case ERR_communication ://communication err
        if(DeviceContrl::CMD_DEVICE_status != cmd)
            messagebox_exec(tr("IDS_ERR_AcquireInformation"));
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
        break;
    case ERR_Password_incorrect :
        if(     (DeviceContrl::CMD_PASSWD_confirmForApply == cmd)
                ||(DeviceContrl::CMD_PASSWD_confirm == cmd)
                ||(DeviceContrl::CMD_PASSWD_confirmForSetPasswd == cmd)
                )
            messagebox_exec(tr("Authentication error, please enter the password again."));
        break;
    case ERR_Printer_busy :
        if(DeviceContrl::CMD_COPY == cmd)
            messagebox_exec(tr("The machine is busy, please try later..."));
        break;
    case ERR_CMD_invalid :
    case ERR_Parameter_invalid :
    case ERR_Do_not_support :
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
    //handle cmd result
    switch(cmd)
    {
    case DeviceContrl::CMD_DEVICE_status:
        cmdResult_getDeviceStatus(err);
        break;
    case DeviceContrl::CMD_PASSWD_confirm:
        cmdResult_passwd_confirmed(err);
        break;
    case DeviceContrl::CMD_PASSWD_confirmForApply:
        cmdResult_passwd_confirmForApply(err);
        break;
    case DeviceContrl::CMD_PASSWD_confirmForSetPasswd:
        cmdResult_passwd_confirmForSetPasswd(err);
        break;
    case DeviceContrl::CMD_PASSWD_set:
        cmdResult_passwd_set(err);
        break;
    case DeviceContrl::CMD_WIFI_apply:
        cmdResult_wifi_apply(err);
        break;
    case DeviceContrl::CMD_WIFI_getAplist:
        cmdResult_wifi_getAplist(err);
        break;
    case DeviceContrl::CMD_WIFI_get:
        cmdResult_wifi_get(err);
        break;
    case DeviceContrl::CMD_WIFI_GetWifiStatus:
    case DeviceContrl::CMD_WIFI_GetWifiStatus_immediately:
        cmdResult_emit_next(err);
        break;
    default:
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
        break;
    }
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
    QEvent::Type type = event->type();
    switch(type){
    case QEvent::Wheel:
        if(qobject_cast<QComboBox*>(obj))
            return true;
        break;
    case QEvent::Show:
        if(obj == ts->pageWidget)
            wifi_init();
        else if(obj == tc->copy)
            emit_cmd(DeviceContrl::CMD_DEVICE_status);
        break;
    case QEvent::Hide:
        if(obj == ui->tab_4)
            passwd_checked = false;
        break;
    case QEvent::MouseButtonPress:
        if (obj == ta->label){
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if(Qt::LeftButton == mouseEvent->button()
                    && mouseEvent->x() > 130 && mouseEvent->y() > 280
                    && mouseEvent->x() < 180 && mouseEvent->y() < 330
                    )        {
                slots_about_update();
            }
//            return true;
        }else if(obj == tc->copies){
            if(!keyboard_copies->isVisible()){
                keyboard_copies->set_num(tc->copies->text().toInt());
                keyboard_copies->show();
            }
        }else if(obj == tc->scaling){
            if(!keyboard_scaling->isVisible() && tc->label_scaling->isEnabled()){
                keyboard_scaling->set_num(tc->scaling->text().remove(QChar('%')).toInt());
                keyboard_scaling->show();
            }
        }
        break;
default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

#include "app/vop_device.h"
extern QMainWindow* gMainWidow;
void MainWidget::updateUi()
{
    QString device_name = device_manager->get_deviceName();

    int model = device_manager->getDeviceModel(device_name);
    ui->tabWidget->hide();
    ui->tabWidget->clear();
    switch(model){
    case VopDevice::Device_3in1:
        ui->tabWidget->addTab(ui->tab_3 ,tr("IDS_Tab_Copy"));
//            ui->tabWidget->addTab(ui->tab_4 ,tr("IDS_Tab_Setting"));
        break;
    case VopDevice::Device_3in1_wifi:
        ui->tabWidget->addTab(ui->tab_3 ,tr("IDS_Tab_Copy"));
        ui->tabWidget->addTab(ui->tab_4 ,tr("IDS_Tab_Setting"));
        break;
    case VopDevice::Device_sfp:
    case VopDevice::Device_sfp_wifi:
    default:
//            ui->tabWidget->addTab(ui->tab_3 ,tr("IDS_Tab_Copy"));
//            ui->tabWidget->addTab(ui->tab_4 ,tr("IDS_Tab_Setting"));
        break;
    }
    ui->tabWidget->addTab(ui->tab_5 ,tr("IDS_Tab_About"));
    ui->tabWidget->setCurrentWidget(ui->tab_5);
    ui->tabWidget->show();

    if(!device_name.isEmpty()){
        QString device_uri = device_manager->getCurrentDeviceURI();
        if(gMainWidow)
            gMainWidow->setWindowTitle(device_name + " - " + device_uri);

        qLog("current device: " + device_name);
        qLog("device uri: "+ device_uri);
    }else{
        if(gMainWidow)
            gMainWidow->setWindowTitle(" ");
        qLog("no device");
    }

}

void MainWidget::on_refresh_clicked()
{
    ui->comboBox_deviceList->clear();
    QStringList printerNames;
    int selected_printer = device_manager->getDeviceList(printerNames);
    if(-1 != selected_printer){//has printer
        ui->tabWidget->setTabEnabled(0 ,true);
        ui->tabWidget->setTabEnabled(1,true);
        ui->comboBox_deviceList->insertItems(0 ,printerNames);
        ui->comboBox_deviceList->setCurrentIndex(selected_printer);
    }
    on_comboBox_deviceList_activated(selected_printer);
}

void MainWidget::on_comboBox_deviceList_activated(int index)
{
    device_manager->selectDevice(index);
    emit signals_deviceChanged(device_manager->get_deviceName());
    updateUi();
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
#include<QUrl>
void MainWidget::slots_about_update()
{
    if(!QDesktopServices::openUrl(QUrl("http://www.lenovo.com"))){
    }
}

/////////////////////////////////////////////////////tab copy/////////////////////////////////////////////////////////////
//static const char* output_size_list[] =
//{
//    QT_TRANSLATE_NOOP_UTF8("TabCopy" ,"IDS_SIZE_Letter") ,//"Letter (8.5 * 11)" ,
//    QT_TRANSLATE_NOOP_UTF8("TabCopy" ,"IDS_SIZE_A4"),//"A4 (210 * 297mm)"
//    QT_TRANSLATE_NOOP_UTF8("TabCopy" ,"IDS_SIZE_A5"),//"A5 (148 * 210mm)"
//    QT_TRANSLATE_NOOP_UTF8("TabCopy" ,"IDS_SIZE_A6"),//"A6 (105 * 148mm)"
//    QT_TRANSLATE_NOOP_UTF8("TabCopy" ,"IDS_SIZE_B5"),//"B5 (182 * 257mm)"
//    QT_TRANSLATE_NOOP_UTF8("TabCopy" ,"IDS_SIZE_B6"),//"B6 (128 * 182mm)"
//    QT_TRANSLATE_NOOP_UTF8("TabCopy" ,"IDS_SIZE_Executive"),//"Executive (7.25 * 105\")"
//    QT_TRANSLATE_NOOP_UTF8("TabCopy" ,"IDS_SIZE_16K"),//"16K (185 * 260mm)"
//};

static const int document_size[] =
{1 ,2 ,4 ,0 ,6};// A4 A5 B5 Letter Executive

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

#define _GetSizeScaling(ds ,os ,index)  ((output_size[os][index] - 8.2) / (output_size[document_size[ds]][index] - 8.2) + 0.005)
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
//    for(i = 0 ;i < sizeof(output_size_list) / sizeof(output_size_list[0]) ;i++){
    for(i = 0 ;i < tc->combo_outputSize->count() ;i++){
//        stringlist_output_size << QApplication::translate("TabCopy" ,output_size_list[i]);
        stringlist_output_size << tc->combo_outputSize->itemText(i);
    }

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

    tc->copy->installEventFilter(this);

    keyboard_scaling = new ScalingSettingKeyboard(this);
    keyboard_scaling->hide();
    tc->scaling->installEventFilter(this);
    connect(keyboard_scaling ,SIGNAL(sendScalingData(QString)) ,this ,SLOT(slots_copy_keyboard(QString)));

    keyboard_copies = new CopiesSettingKeyboard(this);
    keyboard_copies->hide();
    tc->copies->installEventFilter(this);
    connect(keyboard_copies ,SIGNAL(sendCopiesData(QString)) ,this ,SLOT(slots_copy_keyboard(QString)));
}

#define SetWhite(widget) widget->setStyleSheet("background-color:white")
#define SetGray(widget) widget->setStyleSheet("background-color:gray")
#define SetIDCardCopy(widget) widget->setStyleSheet(":enabled:!pressed{background-color:#faaf40}")
#define SetCopy(widget) widget->setStyleSheet(":enabled:!pressed{background-color:#71c2cd}")
#define IsIDCardCopyMode(pCopyPara) (pCopyPara->nUp == 4)
#define SetIDCardCopyMode(pCopyPara) (pCopyPara->nUp = 4)

void MainWidget::updateCopy()
{
    copycmdset copyPara = device_manager->copy_get_para();
    copycmdset* pCopyPara = &copyPara;
    //scaling ui
    tc->scaling->setText(QString("%1%").arg(pCopyPara->scale));
    //copies ui
    tc->copies->setText(QString("%1").arg(pCopyPara->copyNum));
    //density ui
//    SetGray(tc->density1);
    switch(pCopyPara->Density)
    {
    case 1:SetWhite(tc->density2);SetWhite(tc->density3);SetWhite(tc->density4);SetWhite(tc->density5);break;
    case 2:SetGray(tc->density2);SetWhite(tc->density3);SetWhite(tc->density4);SetWhite(tc->density5);break;
    case 3:SetGray(tc->density2);SetGray(tc->density3);SetWhite(tc->density4);SetWhite(tc->density5);break;
    case 4:SetGray(tc->density2);SetGray(tc->density3);SetGray(tc->density4);SetWhite(tc->density5);break;
    default:SetGray(tc->density2);SetGray(tc->density3);SetGray(tc->density4);SetGray(tc->density5);break;
    }
    //scanning mode ui
    pCopyPara->scanMode ?tc->text->setChecked(true):tc->photo->setChecked(true);

    //output size ui
    int value = pCopyPara->paperSize;

    QStringList sl(stringlist_output_size);
    if(1 == pCopyPara->nUp || IsIDCardCopyMode(pCopyPara)){//2in1 hide a6 b6
        sl.removeAt(5);
        sl.removeAt(3);
        if(3 == value || 5 == value){
            value = 0;
        }
        if(value > 4)                value -= 2;
        else if(value > 2)        value --;
    }else if(pCopyPara->nUp) {// 4in1 and 9in1 only letter a4
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        if(value > 1){
            value = 0;
        }
    }
    tc->combo_outputSize->clear();
    tc->combo_outputSize->insertItems(0 ,sl);
    tc->combo_outputSize->setCurrentIndex(value);
    //n in 1 copy ui
    if(!IsIDCardCopyMode(pCopyPara)){
        value =  pCopyPara->paperSize;
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
        tc->combo_nIn1Copy->setCurrentIndex(pCopyPara->nUp);
    }else{
        tc->combo_nIn1Copy->setCurrentIndex(0);
    }
    tc->combo_documentType->setCurrentIndex(pCopyPara->mediaType);
    int docSize =pCopyPara->orgSize;
    if(docSize == 3)    docSize = 0;
    else if(docSize != 4) docSize ++;
    tc->combo_documentSize->setCurrentIndex(docSize);
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

    if(IsIDCardCopyMode(pCopyPara))
    {
        tc->combo_documentSize->setEnabled(false);
        tc->combo_nIn1Copy->setEnabled(false);
        tc->combo_dpi->setEnabled(false);
        SetIDCardCopy(tc->copy);
        tc->bg_scaling->setEnabled(false);
        tc->IDCardCopy->setEnabled(true);
        tc->combo_outputSize->setEnabled(false);
    }
    else
    {
        tc->combo_outputSize->setEnabled(true);
        tc->combo_documentSize->setEnabled(true);
        tc->combo_nIn1Copy->setEnabled(true);
        tc->combo_dpi->setEnabled(true);
        SetCopy(tc->copy);
        if(pCopyPara->nUp)    {
            tc->bg_scaling->setEnabled(false);
            tc->IDCardCopy->setEnabled(false);
        }else{
            tc->bg_scaling->setEnabled(true);
            tc->IDCardCopy->setEnabled(true);
        }
    }
    tc->copy->setEnabled(device_status);
}

void MainWidget::slots_copy_combo(int value)
{
    QObject* sd = sender();
    copycmdset copyPara = device_manager->copy_get_para();
    copycmdset* pCopyPara = &copyPara;
    if(sd == tc->combo_documentType)
        pCopyPara->mediaType = value;
    else if(sd == tc->combo_dpi)
        pCopyPara->dpi = value;
    else if(sd == tc->combo_documentSize) {//disable when IsIDCardCopyMode
        if(0 == value)//letter
            pCopyPara->orgSize = 3;
        else if(4 != value)
            pCopyPara->orgSize = value - 1;
        else
            pCopyPara->orgSize = value;
        if(!pCopyPara->nUp)
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }else if(sd == tc->combo_nIn1Copy){//disable when IsIDCardCopyMode
        pCopyPara->nUp = value;
        if(value)        {
            pCopyPara->scale = 100;
        }else{
                GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
        }
    }    else if(sd == tc->combo_outputSize)    {
        if(1 == pCopyPara->nUp){
            if(value > 3)                value += 2;
            else if(value > 2)        value ++;
        }
        pCopyPara->paperSize = value;
        if(!pCopyPara->nUp)
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }
    device_manager->copy_set_para(pCopyPara);
    updateCopy();
}

void MainWidget::copy_button_IDCardCopy()
{
    copycmdset copyPara = device_manager->copy_get_para();
    copycmdset* pCopyPara = &copyPara;
    if(IsIDCardCopyMode(pCopyPara))    {//ID Card mode
        pCopyPara->nUp = 0;
        GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }else{
        SetIDCardCopyMode(pCopyPara);
        pCopyPara->dpi = 1;//600 * 600
        pCopyPara->scale = 100;
        pCopyPara->paperSize = 1;//A4
//        if(3 == pCopyPara->paperSize || 5 == pCopyPara->paperSize){
//            pCopyPara->paperSize = 0;
//        }
    }
    device_manager->copy_set_para(pCopyPara);
}

void MainWidget::slots_copy_pushbutton()
{
    QObject* sd = sender();
    if(sd == tc->IDCardCopy){//button IDCardCopy click
        copy_button_IDCardCopy();
    }else if(sd == tc->btn_default){
        device_manager->copy_set_defaultPara();
    }else if(sd == tc->scaling_minus){
        copycmdset copyPara = device_manager->copy_get_para();
        copycmdset* pCopyPara = &copyPara;
        pCopyPara->scale --;
    }else if(sd == tc->scaling_plus){
        copycmdset copyPara = device_manager->copy_get_para();
        copycmdset* pCopyPara = &copyPara;
        pCopyPara->scale ++;
        device_manager->copy_set_para(pCopyPara);
    }else if(sd == tc->density_minus){
        copycmdset copyPara = device_manager->copy_get_para();
        copycmdset* pCopyPara = &copyPara;
        pCopyPara->Density --;
        device_manager->copy_set_para(pCopyPara);
    }else if(sd == tc->density_plus){
        copycmdset copyPara = device_manager->copy_get_para();
        copycmdset* pCopyPara = &copyPara;
        pCopyPara->Density ++;
        device_manager->copy_set_para(pCopyPara);
    }else if(sd == tc->copies_minus){
        copycmdset copyPara = device_manager->copy_get_para();
        copycmdset* pCopyPara = &copyPara;
        pCopyPara->copyNum --;
        device_manager->copy_set_para(pCopyPara);
    }else if(sd == tc->copies_plus){
        copycmdset copyPara = device_manager->copy_get_para();
        copycmdset* pCopyPara = &copyPara;
        pCopyPara->copyNum ++;
        device_manager->copy_set_para(pCopyPara);
    }
    updateCopy();
}

void MainWidget::slots_copy_radio(bool checked)
{
    QObject* sd = sender();
    if(sd == tc->photo){
        copycmdset copyPara = device_manager->copy_get_para();
        copycmdset* pCopyPara = &copyPara;
//        if(2 != pCopyPara->scanMode){//not ID Card mode
            pCopyPara->scanMode = !checked;
            device_manager->copy_set_para(pCopyPara);
            updateCopy();
//        }
    }
}

void MainWidget::slots_copy_keyboard(QString str)
{
    copycmdset copyPara = device_manager->copy_get_para();
    copycmdset* pCopyPara = &copyPara;
    QObject* sd = sender();
    if(sd == keyboard_copies){
        pCopyPara->copyNum = str.toInt();
    }else if(sd == keyboard_scaling){
        pCopyPara->scale = str.toInt();
    }
    device_manager->copy_set_para(pCopyPara);
    updateCopy();
}

//////////////////////////tab setting///////////////////
void MainWidget::initializeTabSetting()
{
    ts->setupUi(ui->tab_4);

    ts->stackedWidget->setStyleSheet("QStackedWidget,#pageWidget,#page2Widget{ \
                                min-height:320; \
                                max-height:320; \
                             }");
//*/

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

    QRegExp regexp("^[\\x0020-\\x007e]{1,32}$");
    QValidator *validator = new QRegExpValidator(regexp, this);
    ts->le_ssid->setValidator(validator);

    regexp.setPattern("^[0-9a-zA-Z]{1,32}$");
    QValidator* validator2 =  new QRegExpValidator(regexp, this);
    ts->le_newPassword->setValidator(validator2);
    ts->le_confirmPassword->setValidator(validator2);

    ts->btn_apply_mp->setEnabled(false);

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
    connect(ts->le_newPassword,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ts->le_confirmPassword,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));


    ts->pageWidget->installEventFilter(this);
    ts->cb_encryptionType->installEventFilter(this);
    ts->cb_keyIndex->installEventFilter(this);
    ts->cb_ssid->installEventFilter(this);
    ui->tab_4->installEventFilter(this);

    wifi_update();
    wifi_update_checkbox(ts->checkBox->isChecked());
    ts->checkBox->hide();//checkbox hide
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
    }else if(sd == ts->le_newPassword || sd == ts->le_confirmPassword){
        if(ts->le_newPassword->text().isEmpty() && ts->le_confirmPassword->text().isEmpty()){
            ts->btn_apply_mp->setEnabled(false);
        }else{
            ts->btn_apply_mp->setEnabled(true);
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

void MainWidget::wifi_init()
{
//    emit_cmd(DeviceContrl::CMD_WIFI_get);//for last version
    DeviceApp* device_app = device_manager->deviceApp();
    if(device_app){
//        if(DeviceContrl::CMD_STATUS_COMPLETE == device_app->get_cmdStatus())
        {
            install_next_callback(SLOT(slots_wifi_refreshAplist()));
            emit_cmd(DeviceContrl::CMD_WIFI_get);
        }
    }
}

void MainWidget::slots_wifi_getStatusToRefreshAplist()
{
    if(wifi_getStatus()){
        emit_cmd(DeviceContrl::CMD_WIFI_getAplist);
    }
}

int MainWidget::wifi_getStatus()
{
    int status = 0;
    cmdst_wifi_status wifi_status = device_manager->wifi_getWifiStatus();
    if(1 == wifi_status){//wifi inited
        status = 1;
    }else{
        DeviceApp* device_app = device_manager->deviceApp();
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
    }
    return status;
}

void MainWidget::wifi_getStatusToRefreshAplist()
{
    //only wifi enabled can do
//    emit_cmd(DeviceContrl::CMD_WIFI_get);//for last version
    DeviceApp* device_app = device_manager->deviceApp();
    if(device_app){
//        if(DeviceContrl::CMD_STATUS_COMPLETE == device_app->get_cmdStatus())
        {
            install_next_callback(SLOT(slots_wifi_getStatusToRefreshAplist()));
            emit_cmd(DeviceContrl::CMD_WIFI_GetWifiStatus);
        }
    }
}

void MainWidget::slots_wifi_refreshAplist()
{
    //only wifi enabled can do
    cmdst_wifi_get wifi_para = device_manager->wifi_get_para();
    disconnect(ts->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_checkbox(bool)));
    ts->checkBox->setChecked(wifi_para.wifiEnable & 1 ?true :false);
    connect(ts->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_checkbox(bool)));
    if(wifi_para.wifiEnable & 1){//wifi enabled then refresh aplist
//        wifi_refreshAplist();
        install_next_callback(SLOT(slots_wifi_getStatusToRefreshAplist()));
        emit_cmd(DeviceContrl::CMD_WIFI_GetWifiStatus_immediately);
    }else{
        DeviceApp* device_app = device_manager->deviceApp();
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
    }
}

void MainWidget::wifi_update_checkbox(bool checked)
{
    return;//checkbox hide
    if(checked){
        ts->frame->setEnabled(true);
        ts->btn_apply_ws->setEnabled(wifi_validate_ssidPassword());
    }else{//diable setting
        ts->frame->setEnabled(false);
        ts->btn_apply_ws->setEnabled(false);
    }
}

void MainWidget::slots_wifi_enable()
{
    cmdst_wifi_get wifi_para = device_manager->wifi_get_para();
    //setting wifi enable only
    wifi_para.wifiEnable &= ~1;
    wifi_para.wifiEnable |= ts->checkBox->isChecked() ? 1 : 0;//bit 0
    device_manager->wifi_set_para(&wifi_para);

//    install_next_callback(SLOT(slots_cmd_complete()));
    emit_cmd(DeviceContrl::CMD_WIFI_apply);
}


void MainWidget::install_next_callback(const char *member)
{
    disconnect(SIGNAL(signals_cmd_next()));
    connect(this ,SIGNAL(signals_cmd_next()) ,this ,member);
}

void MainWidget::slots_wifi_checkbox(bool checked)
{
    wifi_update_checkbox(checked);

    DeviceApp* device_app = device_manager->deviceApp();
    if(device_app){
//        if(DeviceContrl::CMD_STATUS_COMPLETE == device_app->get_cmdStatus())
        {
            install_next_callback(SLOT(slots_wifi_enable()));
            emit_cmd(DeviceContrl::CMD_WIFI_get);
        }
    }
}

void MainWidget::cmdResult_passwd_confirmed(int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    if(!err){//no err,ACK
        passwd_checked = true;
//        slots_wifi_apply();//last version
        emit signals_cmd_next();
    }else if(ERR_Password_incorrect == err){//password incorrect
        passwd_checked = false;
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
        wifi_passwd_doConfirm();
    }else{
        device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
    }
}

void MainWidget::slots_passwd_comfirmed()
{
    install_next_callback(SLOT(slots_wifi_getStatusToApply()));
    emit_cmd(DeviceContrl::CMD_WIFI_GetWifiStatus_immediately);
}

#include <QInputDialog>
void MainWidget::wifi_passwd_doConfirm()
{
    if(passwd_checked){
//        emit_cmd(DeviceContrl::CMD_PASSWD_confirmForApply);//last version
        emit_cmd(DeviceContrl::CMD_PASSWD_confirm);
    }else{
        bool ok;
        passwd = QInputDialog::getText(this ,tr("Login") ,tr("Password") ,QLineEdit::Password ,QString() ,&ok);
        if (ok && !passwd.isEmpty()){
            device_manager->passwd_set(passwd.toLatin1());
//            emit_cmd(DeviceContrl::CMD_PASSWD_confirmForApply);//last version
            emit_cmd(DeviceContrl::CMD_PASSWD_confirm);
        }else{
            DeviceApp* device_app = device_manager->deviceApp();
            device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
        }
    }
}

void MainWidget::wifi_passwd_doConfirm(const char *member)
{
    if(passwd_checked){
        install_next_callback(member);
        emit_cmd(DeviceContrl::CMD_PASSWD_confirm);
    }else{
        bool ok;
        passwd = QInputDialog::getText(this ,tr("Login") ,tr("Password") ,QLineEdit::Password ,QString() ,&ok);
        if (ok && !passwd.isEmpty()){
            device_manager->passwd_set(passwd.toLatin1());
            install_next_callback(member);
            emit_cmd(DeviceContrl::CMD_PASSWD_confirm);
        }else{
            DeviceApp* device_app = device_manager->deviceApp();
            device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
        }
    }
}

void MainWidget::slots_wifi_getStatusToApply()
{
    if(wifi_getStatus()){
        wifi_apply();
    }
}

void MainWidget::wifi_apply()
{
    cmdst_wifi_get wifi_para = device_manager->wifi_get_para();
    //setting data then apply
    device_manager->wifi_set_password(&wifi_para ,wifi_password.toLatin1());
    device_manager->wifi_set_ssid(&wifi_para ,wifi_ssid.toLatin1());
    wifi_para.encryption = wifi_encryptionType > 1 ? wifi_encryptionType + 1 :wifi_encryptionType;
    wifi_para.wepKeyId = wifi_wepIndex;
    wifi_para.wifiEnable &= ~1;
    wifi_para.wifiEnable |= ts->checkBox->isChecked() ? 1 : 0;//bit 0
    device_manager->wifi_set_para(&wifi_para);
    emit_cmd(DeviceContrl::CMD_WIFI_apply);
}

void MainWidget::passwd_set_doConfirm()
{
//    if(ts->le_newPassword->text().isEmpty() && ts->le_confirmPassword->text().isEmpty()){
//        messagebox_exec(tr("The new password cannot be empty."));
//    }else
    if(QString::compare(ts->le_newPassword->text() ,ts->le_confirmPassword->text())){
        messagebox_exec(tr("The passwords you entered are different, please try again."));
    }else{
        bool ok;
        passwd = QInputDialog::getText(this ,tr("Login") ,tr("Password") ,QLineEdit::Password ,QString() ,&ok);
        if (ok){
//            if(!passwd.isEmpty()){
                device_manager->passwd_set(passwd.toLatin1());
//                install_next_callback(SLOT(slots_passwd_set()));
                emit_cmd(DeviceContrl::CMD_PASSWD_confirmForSetPasswd);
//            }
        }else{
            DeviceApp* device_app = device_manager->deviceApp();
            device_app->set_cmdStatus(DeviceContrl::CMD_STATUS_COMPLETE);
        }
    }
}

void MainWidget::slots_passwd_set()
{
    device_manager->passwd_set(ts->le_newPassword->text().toLatin1());
    emit_cmd(DeviceContrl::CMD_PASSWD_set);
}

//update all wifi para
void MainWidget::result_wifi_getAplist()
{
    cmdst_wifi_get wifi_para = device_manager->wifi_get_para();
    cmdst_aplist_get aplist = device_manager->wifi_getAplist();
    //wifi enable
//    ts->checkBox->setChecked(wifi_para.wifiEnable & 1 ?true :false);
    //ssid
    QString ssid(wifi_para.ssid);
    if(ssid.count() > 32)
        machine_wifi_ssid = QString(wifi_para.ssid).left(32);
    else
        machine_wifi_ssid = ssid;
    ts->le_ssid->setText(machine_wifi_ssid);
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
            int encryption = aplist.aplist[i].encryption & 7;
            if(encryption > 4) encryption = 4;
            wifi_sw_encryptionType[i] = encryption > 1 ?encryption -1 : encryption;
            if(!ssid.compare(machine_wifi_ssid))
                current_ssid = i;
        }
    }
    for(int i = 0 ;i < NUM_OF_APLIST ;i++){
        qLog(QString().sprintf("display ssid[%d] memory from FW:" ,i));
        qLog(QString().sprintf("%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x"
                               ,aplist.aplist[i].ssid[0] ,aplist.aplist[i].ssid[1],aplist.aplist[i].ssid[2] ,aplist.aplist[i].ssid[3]
                                ,aplist.aplist[i].ssid[4] ,aplist.aplist[i].ssid[5],aplist.aplist[i].ssid[6] ,aplist.aplist[i].ssid[7]));
        qLog(QString().sprintf("%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x"
                               ,aplist.aplist[i].ssid[8] ,aplist.aplist[i].ssid[9],aplist.aplist[i].ssid[10] ,aplist.aplist[i].ssid[11]
                                ,aplist.aplist[i].ssid[12] ,aplist.aplist[i].ssid[13],aplist.aplist[i].ssid[14] ,aplist.aplist[i].ssid[15]));
        qLog(QString().sprintf("%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x"
                               ,aplist.aplist[i].ssid[16] ,aplist.aplist[i].ssid[17],aplist.aplist[i].ssid[18] ,aplist.aplist[i].ssid[19]
                                ,aplist.aplist[i].ssid[20] ,aplist.aplist[i].ssid[21],aplist.aplist[i].ssid[22] ,aplist.aplist[i].ssid[23]));
        qLog(QString().sprintf("%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x,%#.2x"
                               ,aplist.aplist[i].ssid[24] ,aplist.aplist[i].ssid[25],aplist.aplist[i].ssid[26] ,aplist.aplist[i].ssid[27]
                                ,aplist.aplist[i].ssid[28] ,aplist.aplist[i].ssid[29],aplist.aplist[i].ssid[30] ,aplist.aplist[i].ssid[31]
                                ,aplist.aplist[i].ssid[32]));
        qLog(QString().sprintf("ssid[%d] encryptionType from FW:%d" ,i ,aplist.aplist[i].encryption));
    }
    ts->cb_ssid->setCurrentIndex(current_ssid);
    qLog(QString().sprintf("selected ssid:%d encryption:%d" ,current_ssid ,aplist.aplist[current_ssid].encryption));
    //encryption
     ts->cb_encryptionType->setCurrentIndex(wifi_sw_encryptionType[current_ssid]);
    //key index
    ts->cb_keyIndex->setCurrentIndex(wifi_para.wepKeyId % 4);

    //clear passwd
    wifi_sw_password.clear();//Qt4
    ts->le_passphrase->clear();
    ts->le_wepkey->clear();
    wifi_update();
    //update ui else
//    emit ts->cb_ssid->activated(ts->cb_ssid->currentText());
}
