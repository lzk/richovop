/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "ui_tabsetting.h"

#include <QAction>
#include <QMainWindow>
#include <QProgressDialog>
#include<QProgressBar>
#include <QDesktopWidget>

#include "app/log.h"
#include "app/devicecontrol.h"
#include "app/deviceapp.h"
#include "app/devicemanager.h"
#include "app/device.h"
#include "app/linux_api.h"
extern QMainWindow* gMainWindow;

#include "tabcopy.h"
#include "tabsetting.h"
#include "tababout.h"

#include "logo_icon.h"
#include "ricohmessagebox.h"

#include <QSettings>
#include <QDate>
#include <QDebug>
MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    msgBox_toner(NULL),
    msgBox_info(NULL),
    model(Device::Device_3in1_wifi),
    no_space(false),
    donot_cmd_times(0)
{
    device_manager = new DeviceManager(this);
    ui->setupUi(this);
    createActions();
    initializeUi();
    retranslateUi();

    initialize();
    updateUi();
}

MainWidget::~MainWidget()
{
    if(msgBox_toner)    delete msgBox_toner;
    if(msgBox_info)    delete msgBox_info;
    delete ui;
    delete device_manager;
}

void MainWidget::createActions()
{
    //    action_refresh = new QAction(this);
    //    connect(action_refresh ,SIGNAL(triggered()) ,this ,SLOT(on_refresh_clicked()));
}

void MainWidget::initializeUi()
{
//    Logo_icon widget;
//    QPixmap pixmap(widget.size());
//    widget.render(&pixmap);
////    QPixmap pixmap = widget.grab();
//    ui->button_logo->setIcon(QIcon(pixmap));
//    ui->button_logo->setIconSize(QSize(80 ,20));

//    ui->comboBox_deviceList->setView(new QListView);
//    ui->comboBox_deviceList->view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    tab_copy = new TabCopy(this ,device_manager);
    tab_setting = new TabSetting(this ,device_manager);
    tab_about = new TabAbout();
    listWidget = tab_setting->ui->listWidget;

//    ui->tabWidget->clear();
//    delete ui->tab_about;
//    delete ui->tab_copy;
//    delete ui->tab_setting;
//    ui->tabWidget->addTab(tab_copy ,tr("IDS_Tab_Copy"));
//    ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
//    ui->tabWidget->addTab(tab_about ,tr("IDS_Tab_About"));

    connect(this ,SIGNAL(signals_cmd_result(int,int)) ,tab_copy ,SLOT(slots_cmd_result(int,int)));
    connect(this ,SIGNAL(signals_cmd_result(int,int)) ,tab_setting ,SLOT(slots_cmd_result(int,int)));
 }

void MainWidget::retranslateUi()
{
} // retranslateUi

void MainWidget::initialize()
{
    QList<QWidget*> widgets = findChildren<QWidget*>();
    foreach(QWidget* widget ,widgets){
        if(qobject_cast<QAbstractButton*>(widget)
                || qobject_cast<QComboBox*>(widget)
                ){
            widget->setFocusPolicy(Qt::NoFocus);
        }
    }

    ui->comboBox_deviceList->installEventFilter(this);

    connect(&timer ,SIGNAL(timeout()) ,this ,SLOT(slots_timeout()));
//    timer.setInterval(1000);
    timer.start(1000);

    progressDialog = new QProgressDialog(this);
    progressDialog->setWindowFlags(Qt::SplashScreen);

    progressDialog->setRange(0,100);
    progressDialog->setCancelButton(NULL);
    progressDialog->setModal(true);
//    progressDialog->setWindowModality(Qt::WindowModal);

//    msgBox.setWindowTitle(" ");
//    msgBox_info.setWindowTitle(" ");
    msgBox_toner = new TonerMessageBox;
    connect(msgBox_toner ,SIGNAL(buttonClicked(QAbstractButton*))
            ,this ,SLOT(msgBox_toner_button_pressed()));
    msgBox_info = new RicohMessageBox;
}

bool MainWidget::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();
    switch(type){
    case QEvent::Wheel:
        if(qobject_cast<QComboBox*>(obj))
            return true;
        break;
default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

void MainWidget::updateUi()
{
    QString device_name = device_manager->get_deviceName();
    model = device_manager->getDeviceModel(device_name);

    ui->tabWidget->disconnect(this ,SLOT(on_tabWidget_currentChanged(int)));
    ui->tabWidget->clear();
    switch(model){
    case Device::Device_3in1:
//        listWidget->item(0)->setHidden(true);
//#ifdef FUTURE_SUPPORT
//        listWidget->item(4)->setHidden(true);
//#endif
        ui->tabWidget->addTab(tab_copy ,tr("IDS_Tab_Copy"));
//            ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
        break;
    case Device::Device_3in1_wifi:
    case Device::AirPrint_3in1_wifi:
        listWidget->item(0)->setHidden(false);
#ifdef FUTURE_SUPPORT
        listWidget->item(4)->setHidden(false);
#endif
        ui->tabWidget->addTab(tab_copy ,tr("IDS_Tab_Copy"));
        ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
        break;
    case Device::Device_sfp:
//        ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
//        listWidget->item(0)->setHidden(true);
//#ifdef FUTURE_SUPPORT
//        listWidget->item(4)->setHidden(true);
//#endif
        break;
    case Device::Device_sfp_wifi:
    case Device::AirPrint_sfp_wifi:
        listWidget->item(0)->setHidden(false);
#ifdef FUTURE_SUPPORT
        listWidget->item(4)->setHidden(false);
#endif
//            ui->tabWidget->addTab(tab_copy ,tr("IDS_Tab_Copy"));
        ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
        break;
    default:
        break;
    }
    ui->tabWidget->addTab(tab_about ,tr("IDS_Tab_About"));
    ui->tabWidget->setCurrentWidget(tab_about);
    connect(ui->tabWidget ,SIGNAL(currentChanged(int)), this, SLOT(on_tabWidget_currentChanged(int)));

    if(!device_name.isEmpty()){
        QString device_uri = device_manager->getCurrentDeviceURI();
//        setWindowTitle(device_name + " - " + device_uri);
        if(gMainWindow){
            gMainWindow->setWindowTitle(device_name + " - " + device_uri);
        }

    }else{
//        setWindowTitle(" ");
        if(gMainWindow)
            gMainWindow->setWindowTitle(" ");
    }
}

void MainWidget::slots_progressBar(int cmd ,int value)
{
    //value = 0, show progress bar
    if(!value){
        switch(cmd){
        case DeviceContrl::CMD_COPY:
        case DeviceContrl::CMD_PRN_TonerEnd_Set:
        case DeviceContrl::CMD_PRN_PSaveTime_Set:
        case DeviceContrl::CMD_PRN_PowerOff_Set:
        case DeviceContrl::CMD_PASSWD_set_plus:
        case DeviceContrl::CMD_WIFI_apply_plus:
        case DeviceContrl::CMD_IPv4_Set:
        case DeviceContrl::CMD_IPv6_Set:
//            progressDialog->setLabel(new QLabel("\n\n" + tr("IDS_MSG_SetInfo") +"\t\t\t\t\t\n"));
            progressDialog->setLabelText("\n\n" + tr("IDS_MSG_SetInfo") +"\t\t\t\t\t\n");
            progressDialog->setValue(value);
            progressDialog->show();
            progressDialog->move((QApplication::desktop()->width() - progressDialog->width())/2,
                  (QApplication::desktop()->height() - progressDialog->height())/2);
            break;
        case DeviceContrl::CMD_PRN_TonerEnd_Get:
        case DeviceContrl::CMD_WIFI_refresh_plus:
        case DeviceContrl::CMD_PRN_PowerSave_Get:
        case DeviceContrl::CMD_IPv4_Get:
        case DeviceContrl::CMD_IPv6_Get:
//            progressDialog->setLabel(new QLabel("\n\n" + tr("IDS_MSG_GetInfo") +"\t\t\t\t\t\n"));
            progressDialog->setLabelText("\n\n" + tr("IDS_MSG_GetInfo") +"\t\t\t\t\t\n");
            progressDialog->setValue(value);
            progressDialog->show();
            progressDialog->move((QApplication::desktop()->width() - progressDialog->width())/2,
                  (QApplication::desktop()->height() - progressDialog->height())/2);
            break;
        //do not show progress bar
        case DeviceContrl::CMD_DEVICE_status:
        case DeviceContrl::CMD_PRN_GetRegion:
        case DeviceContrl::CMD_Device_GetFirstStatus:
        default:
            break;
        }
    }else{
        if(cmd != DeviceContrl::CMD_DEVICE_status
                && cmd != DeviceContrl::CMD_PRN_GetRegion
                && cmd != DeviceContrl::CMD_Device_GetFirstStatus
                ){
            progressDialog->setValue(value);
        }
    }
}

void MainWidget::slots_timeout()
{
    static int count = 0;
    static bool _no_space = false;
    if(0 == count % 3){
        no_space = device_no_space("/tmp");
        if(no_space){
            _no_space = true;
            on_refresh_clicked();
            messagebox_show(tr("IDS_NOT_ENOUGH_SPACE"));
        }else{
            if(_no_space){
                _no_space = false;
                messagebox_hide();
            }
            if(donot_cmd_times){
               donot_cmd_times --;
            }else{
                switch(model){
                case Device::Device_3in1_wifi:
                case Device::Device_3in1:
                case Device::AirPrint_3in1_wifi:
                    device_manager->emit_cmd_plus(DeviceContrl::CMD_DEVICE_status);
                default:
                    break;
                }
            }
        }
    }

    tab_about->watched_poptime();
    count ++;
    if(count >= 300)
        count = 0;
}

void MainWidget::slots_cmd_result(int cmd ,int err)
{
    C_LOG("cmd return:%s" ,VopProtocol::getErrString(err));
    //cmd complete
    slots_progressBar(cmd ,100);
    //handle err message box
    switch(err){
    case ERR_printer_have_jobs:
    case ERR_sane_scanning:
        if(DeviceContrl::CMD_DEVICE_status != cmd){
            messagebox_exec(tr("IDS_MSG_MachineBusy"));
        }
        break;
    case ERR_communication ://communication err
    case ERR_library:
    case ERR_decode_status:
    case ERR_decode_device:
    case ERR_vop_cannot_support:
        switch(cmd){
        case DeviceContrl::CMD_COPY:
            messagebox_exec(tr("IDS_ERR_Communication"));
            break;
        case DeviceContrl::CMD_PRN_TonerEnd_Set:
        case DeviceContrl::CMD_PRN_PSaveTime_Set:
        case DeviceContrl::CMD_PRN_PowerOff_Set:
        case DeviceContrl::CMD_PASSWD_set_plus:
        case DeviceContrl::CMD_WIFI_apply_plus:
        case DeviceContrl::CMD_IPv4_Set:
        case DeviceContrl::CMD_IPv6_Set:
            if(device_manager->get_passwd_confirmed())
                messagebox_exec(tr("IDS_ERR_Communication"));
            else
                messagebox_exec(tr("IDS_ERR_AcquireInformation"));

            break;
        case DeviceContrl::CMD_PRN_TonerEnd_Get:
        case DeviceContrl::CMD_WIFI_refresh_plus:
        case DeviceContrl::CMD_PRN_PowerSave_Get:
        case DeviceContrl::CMD_IPv4_Get:
        case DeviceContrl::CMD_IPv6_Get:
            messagebox_exec(tr("IDS_ERR_AcquireInformation"));
            break;
        //do not show err msg
        case DeviceContrl::CMD_DEVICE_status:
        case DeviceContrl::CMD_PRN_GetRegion:
        default:
            break;
        }
        break;
    case ERR_wifi_have_not_been_inited:
        messagebox_exec(tr("IDS_WIRELESS_DISABLED_MSG"));
        break;
    case ERR_Password_incorrect :
//        if(     (DeviceContrl::CMD_WIFI_apply_plus == cmd)
//                ||(DeviceContrl::CMD_PASSWD_set_plus == cmd)
//                )
            messagebox_exec(tr("IDS_ERR_Authentication"));
        break;
    case ERR_Printer_busy :
//        if(DeviceContrl::CMD_COPY == cmd)
            messagebox_exec(tr("IDS_MSG_MachineBusy"));
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
    switch(cmd)
    {
    case DeviceContrl::CMD_COPY:
        if(!err)
            donot_cmd_times = 2;
        break;
    case DeviceContrl::CMD_PRN_GetRegion:
//        setEnabled(true);
        ui->widget_devicelist->setEnabled(true);
        ui->tabWidget->setEnabled(true);
        break;
    case DeviceContrl::CMD_Device_GetFirstStatus:
//        qDebug()<<"CMD_Device_GetFirstStatus:"<<err;
        switch(err){
        case STATUS_TonerNearEnd:
            msgBox_toner->messagebox_show(tr("ResStr_Toner_Near_End"));
            break;
        case STATUS_TonerEnd:
            msgBox_toner->messagebox_show(tr("ResStr_Toner_End"));
            break;
        default:
//            msgBox_toner->messagebox_show(tr("ResStr_Toner_Near_End"));
            msgBox_toner->close();
            break;
        }
        break;
    default:
        break;
    }
    //handle cmd result
    emit signals_cmd_result(cmd ,err);
}


void MainWidget::on_refresh_clicked()
{
    _Q_LOG("");
    _Q_LOG("");
    _Q_LOG("refresh device list");
    ui->refresh->setEnabled(false);
    ui->comboBox_deviceList->clear();
    QStringList printerNames;
    int selected_printer = -1;
    if(!no_space){
        selected_printer = device_manager->getDeviceList(printerNames);
        if(-1 != selected_printer){//has printer
            ui->comboBox_deviceList->insertItems(0 ,printerNames);
            ui->comboBox_deviceList->setCurrentIndex(selected_printer);
        }
    }
    on_comboBox_deviceList_activated(selected_printer);
    ui->refresh->setEnabled(true);
}

void MainWidget::on_comboBox_deviceList_activated(int index)
{
    if(device_manager->selectDevice(index)){
    }
    emit signals_deviceChanged(device_manager->get_deviceName());
    updateUi();
//    if(!have_got_region_from_FW){
    if(-1 != index){
        device_manager->emit_cmd_plus(DeviceContrl::CMD_PRN_GetRegion);
//        setEnabled(false);
        ui->widget_devicelist->setEnabled(false);
        ui->tabWidget->setEnabled(false);
    }

    msgBox_toner->close();
    if(!tab_about->get_poptime_checked()){
        QSettings settings;
        QDate date = settings.value("app/message box shown date").toDate();
        if(date != QDate::currentDate()){
            device_manager->emit_cmd_plus(DeviceContrl::CMD_Device_GetFirstStatus);
        }
    }
}

void MainWidget::messagebox_exec(const QString &text)
{
    RicohMessageBox::messagebox_exec(text);
}

void MainWidget::messagebox_show(const QString &text)
{
    msgBox_info->messagebox_show(text);
}

void MainWidget::messagebox_hide()
{
    if(msgBox_info->isVisible())
        msgBox_info->hide();
}

void MainWidget::on_tabWidget_currentChanged(int)
{
    if(tab_setting == ui->tabWidget->currentWidget()){
        if(!listWidget->item(0)->isHidden()){
            if(listWidget->currentRow()){
                listWidget->setCurrentRow(0);
            }else
                tab_setting->on_listWidget_currentRowChanged(0);
        }else
            listWidget->setCurrentRow(1);
    }else if(tab_copy == ui->tabWidget->currentWidget()){
        device_manager->emit_cmd_plus(DeviceContrl::CMD_DEVICE_status);
    }
}
#include <QDesktopServices>
#include<QUrl>
void MainWidget::on_button_logo_clicked()
{
    if(!QDesktopServices::openUrl(QUrl("http://www.ricoh.com/printers/sp150/support/gateway/"))){
    }
}

void MainWidget::msgBox_toner_button_pressed()
{
//    qDebug()<<"button click";
    QSettings settings;
    settings.setValue("app/message box shown date" ,QDate::currentDate());
}

