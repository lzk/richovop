#include "tabsetting.h"
#include "ui_tabsetting.h"

#include "app/log.h"
#include "app/devicecontrol.h"
#include "app/deviceapp.h"
#include "app/devicemanager.h"

#include <QInputDialog>
#include "dialoglogin.h"
#define OLD_CODE 0
TabSetting::TabSetting(MainWidget* widget,DeviceManager* dm ,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabSetting),
    main_widget(widget),
    device_manager(dm),
    disable_emit(false)
{
    ui->setupUi(this);

    ui->stackedWidget->setStyleSheet("#pageWidget{ \
                                min-height:420; \
                                max-height:420; \
                             }");

    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);
    ui->radioButton_searchWifi->setChecked(true);
    ui->searchWifiWidget->show();
    ui->manualSetupWidget->hide();

    wifi_encryptionType = wifi_default_encryptionType;
    wifi_ms_wepIndex = wifi_default_wepIndex;
    wifi_sw_wepIndex = wifi_default_wepIndex;
    wifi_sw_encryptionType[0] = wifi_default_encryptionType;
//    passwd_checked = false;

    QRegExp regexp("^[\\x0020-\\x007e]{1,32}$");
    QValidator *validator = new QRegExpValidator(regexp, this);
    ui->le_ssid->setValidator(validator);

    regexp.setPattern("^[0-9a-zA-Z]{1,32}$");
    QValidator* validator2 =  new QRegExpValidator(regexp, this);
    ui->le_newPassword->setValidator(validator2);
    ui->le_confirmPassword->setValidator(validator2);

    ui->btn_apply_mp->setEnabled(false);
    ui->spinBox_PSaveTime->setValue(1);
    ui->cb_ssid->setView(new QListView);
    ui->cb_ssid->view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(ui->radioButton_searchWifi ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_radiobutton(bool)));
    connect(ui->le_ssid,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->le_wepkey,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->le_passphrase,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->cb_encryptionType,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->cb_keyIndex,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->cb_ssid,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
//    connect(ui->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_checkbox(bool)));
    connect(ui->le_newPassword,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->le_confirmPassword,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));


    ui->pageWidget->installEventFilter(this);
    ui->page3Widget->installEventFilter(this);
    ui->page4Widget->installEventFilter(this);
    ui->cb_encryptionType->installEventFilter(this);
    ui->cb_keyIndex->installEventFilter(this);
    ui->cb_ssid->installEventFilter(this);

    wifi_update();
    wifi_update_checkbox(ui->checkBox->isChecked());
    ui->checkBox->hide();//checkbox hide
    ui->checkBox_powerOff->hide();

}

TabSetting::~TabSetting()
{
    qLog("abc");
    delete ui;
}

bool TabSetting::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();
    switch(type){
    case QEvent::Wheel:
        if(qobject_cast<QComboBox*>(obj))
            return true;
        break;
    case QEvent::Show:
        if(obj == ui->pageWidget)
            on_btn_refresh_clicked();
        else if(obj == ui->page3Widget)
            device_manager->emit_cmd_plus(DeviceContrl::CMD_PRN_TonerEnd_Get);
        else if(obj == ui->page4Widget){
            device_manager->emit_cmd_plus(DeviceContrl::CMD_PRN_PowerSave_Get);
        }
        break;
default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

void TabSetting::hideEvent(QHideEvent * e)
{
//    passwd_checked = false;
    device_manager->set_passwd_confirmed(false);
    QWidget::hideEvent(e);
}

void TabSetting::wifi_update_encryptionType()
{
    switch(wifi_encryptionType)
    {
    case 1://wep
        ui->passphraseWidget->hide();
        ui->wepWidget->show();
        break;
    case 2://wpa2-psk-aes
    case 3://mixed mode psk
        ui->passphraseWidget->show();
        ui->wepWidget->hide();
        break;
    default:
        ui->passphraseWidget->hide();
        ui->wepWidget->hide();
        break;
    }
}

void TabSetting::slots_wifi_radiobutton(bool checked)
{
    QObject* sd = sender();
    if(sd == ui->radioButton_searchWifi){
        if(checked){//show wifi setup
            ui->searchWifiWidget->show();
            ui->manualSetupWidget->hide();
            //search wifi password
            ui->le_wepkey->setText(wifi_sw_password);
            ui->le_passphrase->setText(wifi_sw_password);
            ui->cb_keyIndex->setCurrentIndex(wifi_sw_wepIndex);
        }else{//show manual setup
            ui->searchWifiWidget->hide();
            ui->manualSetupWidget->show();
            //manual setup password
            ui->le_wepkey->setText(wifi_ms_password);
            ui->le_passphrase->setText(wifi_ms_password);
            ui->cb_keyIndex->setCurrentIndex(wifi_ms_wepIndex);
        }
        //clear password when radio button emit
//        ui->le_wepkey->clear();
//        ui->le_passphrase->clear();
        wifi_update();
    }
}

void TabSetting::wifi_update()
{
    wifi_update_Data();
    wifi_update_encryptionType();
    ui->btn_apply_ws->setEnabled(wifi_validate_ssidPassword());
}

void TabSetting::wifi_update_Data()
{
    if(ui->radioButton_searchWifi->isChecked()){
        wifi_ssid = ui->cb_ssid->currentText();
        int index = ui->cb_ssid->currentIndex();
        wifi_encryptionType = wifi_sw_encryptionType[index < 0 ?0 :index];
        wifi_wepIndex = wifi_sw_wepIndex;
    }else{
        wifi_encryptionType = ui->cb_encryptionType->currentIndex();
        wifi_ssid = ui->le_ssid->text();
        wifi_wepIndex = wifi_ms_wepIndex;
    }
    if(1 == wifi_encryptionType){
        wifi_password = ui->le_wepkey->text();
    }else{
        wifi_password = ui->le_passphrase->text();//not use when encryptionType=0
    }
}

void TabSetting::slots_wifi_textChanged(const QString &arg1)
{
    QObject* sd = sender();
    if(sd == ui->le_passphrase){
        if(ui->radioButton_searchWifi->isChecked()){
            wifi_sw_password = arg1;
        }else{
            wifi_ms_password = arg1;
        }
        wifi_update();
    }else if(sd == ui->le_wepkey){
        if(ui->radioButton_searchWifi->isChecked()){
            wifi_sw_password = arg1;
        }else{
            wifi_ms_password = arg1;
        }
        wifi_update();
    }else if(sd == ui->le_ssid){
        wifi_update();
    }else  if(sd == ui->cb_encryptionType){
//        wifi_ms_password.clear();
//        ui->le_passphrase->clear();
//        ui->le_wepkey->clear();
        wifi_update();
    }else if(sd == ui->cb_ssid){
        wifi_sw_password.clear();
        ui->le_passphrase->clear();
        ui->le_wepkey->clear();
        wifi_update();
    }else if(sd == ui->cb_keyIndex){
        if(ui->radioButton_searchWifi->isChecked()){
            wifi_sw_wepIndex = ui->cb_keyIndex->currentIndex();
        }else{
            wifi_ms_wepIndex = ui->cb_keyIndex->currentIndex();
        }
    }else if(sd == ui->le_newPassword || sd == ui->le_confirmPassword){
        if(ui->le_newPassword->text().isEmpty() && ui->le_confirmPassword->text().isEmpty()){
            ui->btn_apply_mp->setEnabled(false);
        }else{
            ui->btn_apply_mp->setEnabled(true);
        }
    }
}

bool TabSetting::wifi_validate_ssidPassword()
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

void TabSetting::wifi_update_checkbox(bool checked)
{
    return;//checkbox hide
    if(checked){
        ui->frame->setEnabled(true);
        ui->btn_apply_ws->setEnabled(wifi_validate_ssidPassword());
    }else{//diable setting
        ui->frame->setEnabled(false);
        ui->btn_apply_ws->setEnabled(false);
    }
}
#if OLD_CODE
void TabSetting::slots_wifi_enable()
{
    cmdst_wifi_get wifi_para = device_manager->wifi_get_para();
    //setting wifi enable only
    wifi_para.wifiEnable &= ~1;
    wifi_para.wifiEnable |= ui->checkBox->isChecked() ? 1 : 0;//bit 0
    device_manager->wifi_set_para(&wifi_para);

//    install_next_callback(SLOT(slots_cmd_complete()));
    device_manager->emit_cmd(DeviceContrl::CMD_WIFI_apply);
}


void TabSetting::slots_wifi_checkbox(bool checked)
{
    wifi_update_checkbox(checked);

    DeviceApp* device_app = device_manager->deviceApp();
    if(device_app){
//        if(DeviceContrl::CMD_STATUS_COMPLETE == device_app->get_cmdStatus())
        {
            install_next_callback(SLOT(slots_wifi_enable()));
            device_manager->emit_cmd(DeviceContrl::CMD_WIFI_get);
        }
    }
}
#endif
//update all wifi para
void TabSetting::result_wifi_getAplist()
{
    cmdst_wifi_get wifi_para = device_manager->wifi_get_para();
    cmdst_aplist_get aplist = device_manager->wifi_getAplist();
    //wifi enable
//    disconnect(ui->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_checkbox(bool)));
//    ui->checkBox->setChecked(wifi_para.wifiEnable & 1 ?true :false);
//    connect(ui->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_checkbox(bool)));
    //ssid
    QString ssid(wifi_para.ssid);
    if(ssid.count() > 32)
        machine_wifi_ssid = QString(wifi_para.ssid).left(32);
    else
        machine_wifi_ssid = ssid;
    ui->le_ssid->setText(machine_wifi_ssid);
    //aplist
    ui->cb_ssid->clear();
    int current_ssid = -1;
    for(int i = 0 ;i < NUM_OF_APLIST ;i++){
//                QString ssid((char*)&aplist.aplist[i]);
        QString ssid = QString(aplist.aplist[i].ssid).left(32);
        if(ssid.isEmpty()){
            break;
        }else{
            ui->cb_ssid->addItem( ssid);
            int encryption = aplist.aplist[i].encryption & 7;
            if(encryption > 4) encryption = 4;
            wifi_sw_encryptionType[i] = encryption > 1 ?encryption -1 : encryption;
            if(0x80 == (0x80 & aplist.aplist[i].encryption))//FW LShell spec 0811
                if(!ssid.compare(machine_wifi_ssid))
                    current_ssid = i;
        }
    }
    if(-1 == current_ssid){
        for(int i = 0 ;i < NUM_OF_APLIST ;i++){
    //                QString ssid((char*)&aplist.aplist[i]);
            QString ssid = QString(aplist.aplist[i].ssid).left(32);
            if(ssid.isEmpty()){
                break;
            }else{
                if(!ssid.compare(machine_wifi_ssid))//before FW LShell spec 0811
                    current_ssid = i;
            }
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
    if(-1 != current_ssid){
        ui->cb_ssid->setCurrentIndex(current_ssid);
        qLog(QString().sprintf("selected ssid:%d encryption:%d" ,current_ssid ,aplist.aplist[current_ssid].encryption));
        //encryption
         ui->cb_encryptionType->setCurrentIndex(wifi_sw_encryptionType[current_ssid]);
         //key index
         qLog(QString().sprintf("wepKeyId:%d" ,wifi_para.wepKeyId));
         if(wifi_para.wepKeyId){
             wifi_ms_wepIndex = (wifi_para.wepKeyId - 1) % 4;
             wifi_sw_wepIndex = wifi_ms_wepIndex;
         }else{
             wifi_ms_wepIndex = wifi_default_wepIndex;
             wifi_sw_wepIndex = wifi_ms_wepIndex;
         }
         ui->cb_keyIndex->setCurrentIndex(wifi_sw_wepIndex);
    }else{
        qLog("no ssid");
        //encryption
         ui->cb_encryptionType->setCurrentIndex(wifi_default_encryptionType);
         //key index
         wifi_ms_wepIndex = wifi_default_wepIndex;
         wifi_sw_wepIndex = wifi_ms_wepIndex;
         ui->cb_keyIndex->setCurrentIndex(wifi_default_wepIndex);
    }

    //clear passwd
    wifi_sw_password.clear();//Qt4
    ui->le_passphrase->clear();
    ui->le_wepkey->clear();
    wifi_update();
    //update ui else
//    emit ui->cb_ssid->activated(ui->cb_ssid->currentText());
}

void TabSetting::on_spinBox_PSaveTime_valueChanged(int arg1)
{
    ui->pushButton_psave_minus->setEnabled(false);
    if(arg1 == 1){
        ui->pushButton_psave_plus->setEnabled(true);
        ui->pushButton_psave_minus->setEnabled(false);
    }else if(arg1 == 30){
        ui->pushButton_psave_plus->setEnabled(false);
        ui->pushButton_psave_minus->setEnabled(true);
    }else{
        ui->pushButton_psave_plus->setEnabled(true);
        ui->pushButton_psave_minus->setEnabled(true);
    }
    if(!disable_emit){
        if (device_manager->get_passwd_confirmed() || setting_confirmPasswd()){
            cmdst_PSave_time pSave_time = arg1;
            device_manager->printer_setPSaveTime(&pSave_time);
            device_manager->emit_cmd_plus(DeviceContrl::CMD_PRN_PSaveTime_Set);
        }
    }
}

void TabSetting::on_checkBox_powerOff_toggled(bool checked)
{
    if (device_manager->get_passwd_confirmed() || setting_confirmPasswd()){
        cmdst_powerOff_time powerOff_time = checked ? 1 : 0;
        device_manager->printer_setPowerOffTime(&powerOff_time);
        device_manager->emit_cmd_plus(DeviceContrl::CMD_PRN_PowerOff_Set);
    }
}

void TabSetting::on_checkBox_tonerEnd_toggled(bool checked)
{
    if(!disable_emit){
        if (device_manager->get_passwd_confirmed() || setting_confirmPasswd()){
            cmdst_tonerEnd tonerEnd = checked ? 1 : 0;
            device_manager->printer_setTonerEnd(&tonerEnd);
            device_manager->emit_cmd_plus(DeviceContrl::CMD_PRN_TonerEnd_Set);
        }
    }
}

void TabSetting::slots_cmd_result(int cmd ,int err)
{
    DeviceApp* device_app = device_manager->deviceApp();
    if(!device_app)
        return;
    //handle cmd result
    switch(cmd)
    {
    case DeviceContrl::CMD_PRN_TonerEnd_Get:
        if(!err){
        cmdst_tonerEnd para = device_manager->printer_getTonerEnd();
        disable_emit = true;
        if(para)
            ui->checkBox_tonerEnd->setChecked(true);
        else
            ui->checkBox_tonerEnd->setChecked(false);
        }
        disable_emit = false;
        break;

    case DeviceContrl::CMD_WIFI_apply_plus:
        if(!err){//no err,ACK
            //clear passwd
            wifi_ms_password.clear();
            wifi_sw_password.clear();
            ui->le_passphrase->clear();
            ui->le_wepkey->clear();
            wifi_update();
        }else if(ERR_Password_incorrect == err){//password incorrect
            on_btn_apply_ws_clicked();
        }
        break;
    case DeviceContrl::CMD_PASSWD_set_plus:
        if(!err){//no err,ACK
            ui->le_confirmPassword->clear();
            ui->le_newPassword->clear();
        }else if(ERR_Password_incorrect == err){//password incorrect
            on_btn_apply_mp_clicked();
        }
        break;
    case DeviceContrl::CMD_WIFI_refresh_plus:
        if(!err)
            result_wifi_getAplist();
        break;
    case DeviceContrl::CMD_PRN_PowerSave_Get:
        if(!err){
            cmdst_PSave_time para = device_manager->printer_getPSaveTime();
            disable_emit = true;
            ui->spinBox_PSaveTime->setValue(para);
            disable_emit = false;

//            cmdst_powerOff_time para = device_manager->printer_getPowerOffTime();
//            if(para)
//                ui->checkBox_powerOff->setChecked(true);
//            else
//                ui->checkBox_powerOff->setChecked(false);
        }
        break;
    case DeviceContrl::CMD_DEVICE_status:
    default:
        break;
    }
}

void TabSetting::on_btn_apply_ws_clicked()
{
    if (device_manager->get_passwd_confirmed() || setting_confirmPasswd()){
        cmdst_wifi_get wifi_para = device_manager->wifi_get_para();
        //setting data then apply
        device_manager->wifi_set_password(&wifi_para ,wifi_password.toLatin1());
        device_manager->wifi_set_ssid(&wifi_para ,wifi_ssid.toLatin1());
        wifi_para.encryption = wifi_encryptionType > 1 ? wifi_encryptionType + 1 :wifi_encryptionType;
        wifi_para.wepKeyId = wifi_wepIndex + 1;
        wifi_para.wifiEnable &= ~1;
        wifi_para.wifiEnable |= 1;//bit 0
//        wifi_para.wifiEnable |= ui->checkBox->isChecked() ? 1 : 0;//bit 0
        device_manager->wifi_set_para(&wifi_para);
        device_manager->emit_cmd_plus(DeviceContrl::CMD_WIFI_apply_plus);
    }
}

void TabSetting::on_btn_apply_mp_clicked()
{
    if(!QString::compare(ui->le_newPassword->text() ,ui->le_confirmPassword->text())){
        if (setting_confirmPasswd()){
            device_manager->set_tmp_passwd(ui->le_newPassword->text().toLatin1());
            device_manager->emit_cmd_plus(DeviceContrl::CMD_PASSWD_set_plus);
        }
    }
}

void TabSetting::on_btn_refresh_clicked()
{
    device_manager->emit_cmd_plus(DeviceContrl::CMD_WIFI_refresh_plus);
}

bool TabSetting::setting_confirmPasswd()
{
        bool ok;
        passwd = DialogLogin::getPasswd(this ,&ok);
        if (ok && !passwd.isEmpty()){
            device_manager->passwd_set(passwd.toLatin1());
        }else{
            ok = false;
        }
        return ok;
}
