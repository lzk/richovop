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
#include "app/vop_device.h"
extern QMainWindow* gMainWidow;

#include "tabcopy.h"
#include "tabsetting.h"
#include "tababout.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    model(VopDevice::Device_3in1_wifi)
{
    device_manager = new DeviceManager(this);
    ui->setupUi(this);
    createActions();
    initializeUi();
    retranslateUi();

    initialize();
    QList<QWidget*> widgets = findChildren<QWidget*>();
    foreach(QWidget* widget ,widgets){
        if(qobject_cast<QAbstractButton*>(widget)
                || qobject_cast<QComboBox*>(widget)
                ){
            widget->setFocusPolicy(Qt::NoFocus);
        }
    }
}

MainWidget::~MainWidget()
{
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
    ui->comboBox_deviceList->setView(new QListView);
    ui->comboBox_deviceList->view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    tab_copy = new TabCopy(this ,device_manager);
    tab_setting = new TabSetting(this ,device_manager);
    tab_about = new TabAbout();
    listWidget = tab_setting->ui->listWidget;

    ui->tabWidget->clear();
    delete ui->tab_about;
    delete ui->tab_copy;
    delete ui->tab_setting;
    ui->tabWidget->addTab(tab_copy ,tr("IDS_Tab_Copy"));
    ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
    ui->tabWidget->addTab(tab_about ,tr("IDS_Tab_About"));

    qLog("setting parent is " + tab_setting->parent()->objectName());
    tab_copy->installEventFilter(this);
    connect(this ,SIGNAL(signals_cmd_result(int,int)) ,tab_copy ,SLOT(slots_cmd_result(int,int)));
    connect(this ,SIGNAL(signals_cmd_result(int,int)) ,tab_setting ,SLOT(slots_cmd_result(int,int)));
 }

void MainWidget::retranslateUi()
{
} // retranslateUi

void MainWidget::initialize()
{
    connect(&timer ,SIGNAL(timeout()) ,this ,SLOT(slots_timeout()));
//    timer.setInterval(1000);
    timer.start(1000);

    progressDialog = new QProgressDialog(this);
    progressDialog->setWindowFlags(Qt::SplashScreen);

    progressDialog->setCancelButton(NULL);
    progressDialog->setModal(true);

    msgBox.setWindowTitle(" ");
    msgBox_info.setWindowTitle(" ");

}

bool MainWidget::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();
    switch(type){
    case QEvent::Wheel:
        if(qobject_cast<QComboBox*>(obj))
            return true;
        break;
    case QEvent::Show:
        if(obj == tab_copy)
            device_manager->emit_cmd_plus(DeviceContrl::CMD_DEVICE_status);
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
    ui->tabWidget->hide();//revoid cmd
    ui->tabWidget->clear();
    switch(model){
    case VopDevice::Device_3in1:
        listWidget->item(0)->setHidden(false);
        listWidget->setCurrentRow(0);
        ui->tabWidget->addTab(tab_copy ,tr("IDS_Tab_Copy"));
//            ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
        break;
    case VopDevice::Device_3in1_wifi:
        listWidget->item(0)->setHidden(false);
        listWidget->setCurrentRow(0);
        ui->tabWidget->addTab(tab_copy ,tr("IDS_Tab_Copy"));
        ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
        break;
    case VopDevice::Device_sfp:
        ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
        listWidget->item(0)->setHidden(true);
        listWidget->setCurrentRow(1);
        break;
    case VopDevice::Device_sfp_wifi:
        listWidget->item(0)->setHidden(false);
        listWidget->setCurrentRow(0);
//            ui->tabWidget->addTab(tab_copy ,tr("IDS_Tab_Copy"));
        ui->tabWidget->addTab(tab_setting ,tr("IDS_Tab_Setting"));
        break;
    default:
        break;
    }
    ui->tabWidget->addTab(tab_about ,tr("IDS_Tab_About"));
    ui->tabWidget->show();//revoid cmd

    ui->tabWidget->setCurrentWidget(tab_about);
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
        default:
            break;
        }
    }else{
        if(cmd != DeviceContrl::CMD_DEVICE_status){
            progressDialog->setValue(value);
        }
    }
}

void MainWidget::slots_timeout()
{
    static int count = 0;

    if(0 == count % 5
            && (model == VopDevice::Device_3in1_wifi || model == VopDevice::Device_3in1))
        device_manager->emit_cmd_plus(DeviceContrl::CMD_DEVICE_status);

    count ++;
    if(count >= 100)
        count = 0;
}

void MainWidget::slots_cmd_result(int cmd ,int err)
{
    qLog(QString("cmd return:") + VopProtocol::getErrString(err));
    //cmd complete
    slots_progressBar(cmd ,100);
    //handle err message box
    switch(err){
    case ERR_communication ://communication err
        if(DeviceContrl::CMD_DEVICE_status != cmd)
            messagebox_exec(tr("IDS_ERR_AcquireInformation"));
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
    //handle cmd result
    emit signals_cmd_result(cmd ,err);
}


void MainWidget::on_refresh_clicked()
{
    ui->comboBox_deviceList->clear();
    QStringList printerNames;
    int selected_printer = device_manager->getDeviceList(printerNames);
    if(-1 != selected_printer){//has printer
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
