#include "tabsetting.h"
#include "ui_tabsetting.h"

#include "app/log.h"
#include "app/devicecontrol.h"
#include "app/deviceapp.h"
#include "app/devicemanager.h"

#include <QInputDialog>
#include "dialoglogin.h"
#include <QDebug>
#define OLD_CODE 0
TabSetting::TabSetting(MainWidget* widget,DeviceManager* dm ,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabSetting),
    main_widget(widget),
    device_manager(dm),
    disable_emit(false)
{
    ui->setupUi(this);

    init_wifi();

    //init password
    QRegExp regexp;
    regexp.setPattern("^[0-9a-zA-Z]{1,32}$");
    QValidator* validator2 =  new QRegExpValidator(regexp, this);
    ui->le_newPassword->setValidator(validator2);
    ui->le_confirmPassword->setValidator(validator2);

    connect(ui->le_newPassword,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->le_confirmPassword,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));

    //init power save
    ui->checkBox_powerOff->hide();

    init_ip();

}

TabSetting::~TabSetting()
{
    delete ui;
}

void TabSetting::init_wifi()
{
    wifi_ms_encryptionType = wifi_default_encryptionType;
    for(int i = 0 ;i < NUM_OF_APLIST ;i++)
        wifi_aplist_encryptionType[i] = wifi_default_encryptionType;
    wifi_encryptionType = wifi_default_encryptionType;
    wifi_sw_encryptionType = wifi_default_encryptionType;

    wifi_ms_wepIndex = wifi_default_wepIndex;
    wifi_sw_wepIndex = wifi_default_wepIndex;
    wifi_wepIndex = wifi_default_wepIndex;

    ui->stackedWidget->setStyleSheet("#pageWidget{ \
                                min-height:420; \
                                max-height:420; \
                             }");

    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);
    ui->radioButton_searchWifi->setChecked(true);
    ui->searchWifiWidget->show();
    ui->manualSetupWidget->hide();

    QRegExp regexp("^[\\x0020-\\x007e]{1,32}$");
    QValidator *validator = new QRegExpValidator(regexp, this);
    ui->le_ssid->setValidator(validator);

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

    ui->pageWidget->installEventFilter(this);
    ui->page3Widget->installEventFilter(this);
    ui->page4Widget->installEventFilter(this);
    ui->cb_encryptionType->installEventFilter(this);
    ui->cb_keyIndex->installEventFilter(this);
    ui->cb_ssid->installEventFilter(this);

    wifi_update();
//    wifi_update_checkbox(ui->checkBox->isChecked());
    ui->checkBox->hide();//checkbox hide
}

void TabSetting::init_ip()
{
    ui->ipv6_sb_manualAddressMask->setMinimum(0x80000000);
    ui->ipv6_sb_manualAddressMask->setMaximum(0x7fffffff);
    ui->page5Widget->installEventFilter(this);

    ui->listWidget->item(4)->setHidden(true);
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
        }else if(obj == ui->page5Widget){
            if(ui->ipa_rb_ipv4->isChecked()){
                on_ipa_rb_ipv4_toggled(true);
            }else{
                ui->ipa_rb_ipv4->setChecked(true);
            }
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

void TabSetting::wifi_update_encryptionType(int et)
{
    switch(et)
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

void TabSetting::wifi_update_encryptionType()
{
    wifi_update_encryptionType(wifi_encryptionType);
}

void TabSetting::slots_wifi_radiobutton(bool checked)
{
    QObject* sd = sender();
    if(sd == ui->radioButton_searchWifi){
        if(checked){
            //show wifi setup
            ui->searchWifiWidget->show();
            ui->manualSetupWidget->hide();
            //search wifi password
            ui->le_wepkey->setText(wifi_sw_password);
            ui->le_passphrase->setText(wifi_sw_password);
            //wep key index
            ui->cb_keyIndex->setCurrentIndex(wifi_sw_wepIndex);
        }else{
            //show manual setup
            ui->searchWifiWidget->hide();
            ui->manualSetupWidget->show();
            //manual setup password
            ui->le_wepkey->setText(wifi_ms_password);
            ui->le_passphrase->setText(wifi_ms_password);
            //wep key index
            ui->cb_keyIndex->setCurrentIndex(wifi_ms_wepIndex);
            //encryption type
            ui->cb_encryptionType->setCurrentIndex(wifi_ms_encryptionType);
        }

        //clear password when radio button emit
//        ui->le_wepkey->clear();
//        ui->le_passphrase->clear();

        wifi_update();
    }
}
void TabSetting::wifi_update(QString ssid ,QString passwd ,int et)
{
    wifi_update_encryptionType(et);
    ui->btn_apply_ws->setEnabled(wifi_validate_ssidPassword(ssid ,passwd ,et));
}
void TabSetting::wifi_update()
{
    wifi_update_Data();
    wifi_update(wifi_ssid ,wifi_password ,wifi_encryptionType);
}

void TabSetting::wifi_update_Data()
{
    if(ui->radioButton_searchWifi->isChecked()){
        wifi_ssid = ui->cb_ssid->currentText();
        int index = ui->cb_ssid->currentIndex();
        if(index != -1){
            wifi_encryptionType = wifi_aplist_encryptionType[index];
        }else{
            wifi_encryptionType = wifi_default_encryptionType;
        }
        wifi_wepIndex = wifi_sw_wepIndex;
    }else{
        wifi_encryptionType = wifi_ms_encryptionType;
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
        wifi_ms_encryptionType = ui->cb_encryptionType->currentIndex();
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
        if(ui->le_newPassword->text().isEmpty() || ui->le_confirmPassword->text().isEmpty()){
            ui->btn_apply_mp->setEnabled(false);
        }else{
            ui->btn_apply_mp->setEnabled(true);
        }
    }
}

bool TabSetting::wifi_validate_ssidPassword(QString ssid ,QString passwd ,int et)
{
    bool validPattern = false;
    if(et){
        QRegExp regexp;
        if(1 == et)
        {
            regexp.setPattern("^(?:.{5,5}|.{13,13}|[0-9a-fA-F]{10,10}|[0-9a-fA-F]{26,26})$");
        }else{
            regexp.setPattern("^(?:.{8,63}|[0-9a-fA-F]{64,64})$");
        }
        if(!ssid.isEmpty() && !passwd.isEmpty() && -1 != regexp.indexIn(passwd))
            validPattern = true;
    }else{
        if(!ssid.isEmpty())
             validPattern = true;
    }
    return validPattern;
}

bool TabSetting::wifi_validate_ssidPassword()
{
    return wifi_validate_ssidPassword(wifi_ssid ,wifi_password ,wifi_encryptionType);
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
    QString ssid = QString(wifi_para.ssid).left(32);
    ui->le_ssid->setText(ssid);
    wifi_ssid = QString(wifi_para.ssid).left(32);
    //ssid aplist
    ui->cb_ssid->clear();
    int current_ssid = -1;
    for(int i = 0 ;i < NUM_OF_APLIST ;i++){
        ssid = QString(aplist.aplist[i].ssid).left(32);
        if(ssid.isEmpty()){
            break;
        }else{
            ui->cb_ssid->addItem( ssid);
            int encryption = aplist.aplist[i].encryption & 7;
            if(encryption > 4) encryption = 4;
            wifi_aplist_encryptionType[i] = encryption > 1 ?encryption -1 : encryption;
            if(0x80 == (0x80 & aplist.aplist[i].encryption)){//FW LShell spec 0811
                qLog("use bit 7 after FW LShell spec 0811");
                if(!ssid.compare(wifi_ssid))
                    current_ssid = i;
            }
            qLog("ssid:"+ ssid +QString().sprintf(" index:%d encryptionType:%d" ,i ,aplist.aplist[i].encryption));
        }
    }
    if(-1 == current_ssid){
        for(int i = 0 ;i < NUM_OF_APLIST ;i++){
            QString ssid = QString(aplist.aplist[i].ssid).left(32);
            if(ssid.isEmpty()){
                break;
            }else{
                qLog("do not use bit 7 before FW LShell spec 0811");
                if(!ssid.compare(wifi_ssid))
                    current_ssid = i;
            }
        }
    }
#if 0
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
#endif
    if(-1 != current_ssid){
        wifi_encryptionType = wifi_aplist_encryptionType[current_ssid];//aplist.aplist[current_ssid].encryption;
        qLog(QString().sprintf("selected encryption[%d]:%d" ,current_ssid ,wifi_encryptionType));
        qLog(QString().sprintf("wifi_para.encryption:%d" ,wifi_para.encryption));
        qLog(QString().sprintf("wepKeyId from FW:%d" ,wifi_para.wepKeyId));
        if(wifi_para.wepKeyId){
            wifi_wepIndex = (wifi_para.wepKeyId - 1) % 4;
        }else{
            wifi_wepIndex = wifi_default_wepIndex;
        }
    }else{
        qLog("no ssid");
        wifi_encryptionType = wifi_default_encryptionType;
        wifi_wepIndex = wifi_default_wepIndex;
    }
    //encryption
    wifi_ms_encryptionType = wifi_encryptionType;
    wifi_sw_encryptionType = wifi_encryptionType;
     ui->cb_encryptionType->setCurrentIndex(wifi_encryptionType);
    //wep key index
     wifi_ms_wepIndex = wifi_wepIndex;
     wifi_sw_wepIndex = wifi_wepIndex;
     ui->cb_keyIndex->setCurrentIndex(wifi_wepIndex);
#if 0
    if(-1 != current_ssid){
        ui->cb_ssid->setCurrentIndex(current_ssid);
        qLog(QString().sprintf("selected ssid:%d encryption:%d" ,current_ssid ,aplist.aplist[current_ssid].encryption));
        //encryption
         ui->cb_encryptionType->setCurrentIndex(wifi_aplist_encryptionType[current_ssid]);
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
#endif
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
    case DeviceContrl::CMD_IPv4_Get:
        if(!err){
        net_info_st info = device_manager->net_getIpv4info();
        if(4 == info.IPAddressMode)
            ui->ipa_rb_stable->setChecked(true);
        else
            ui->ipa_rb_autoGet->setChecked(true);
        ui->ipa_sb_ipAddress->setValue(info.IPAddress[0]);
        ui->ipa_sb_ipAddress_2->setValue(info.IPAddress[1]);
        ui->ipa_sb_ipAddress_3->setValue(info.IPAddress[2]);
        ui->ipa_sb_ipAddress_4->setValue(info.IPAddress[3]);
        ui->ipa_sb_subnetMask->setValue(info.SubnetMask[0]);
        ui->ipa_sb_subnetMask_2->setValue(info.SubnetMask[1]);
        ui->ipa_sb_subnetMask_3->setValue(info.SubnetMask[2]);
        ui->ipa_sb_subnetMask_4->setValue(info.SubnetMask[3]);
        ui->ipa_sb_gateway->setValue(info.GatewayAddress[0]);
        ui->ipa_sb_gateway_2->setValue(info.GatewayAddress[1]);
        ui->ipa_sb_gateway_3->setValue(info.GatewayAddress[2]);
        ui->ipa_sb_gateway_4->setValue(info.GatewayAddress[3]);
        }
        break;
    case DeviceContrl::CMD_IPv6_Get:
        if(!err){
            net_ipv6_st info = device_manager->net_getIpv6info();
            ui->ipv6_cb_dhcp->setChecked(!!info.DHCPv6);
            ui->ipv6_cb_UMA->setChecked(!!info.UseManualAddress);
            ui->ipv6_le_manualAddress->setText(QString(info.ManualAddress).left(40));
            ui->ipv6_sb_manualAddressMask->setValue(info.ManualMask);
            ui->ipv6_le_manualGatewayAddress->setText(QString(info.IPv6ManualGatewayAddress).left(40));
            ui->ipv6_lb_SA1->setText(QString(info.StatelessAddress1).left(40));
            ui->ipv6_lb_SA2->setText(QString(info.StatelessAddress2).left(40));
            ui->ipv6_lb_SA3->setText(QString(info.StatelessAddress3).left(40));
            ui->ipv6_lb_AGA->setText(QString(info.AutoGatewayAddress).left(40));
            ui->ipv6_lb_ASA->setText(QString(info.AutoStatefulAddress).left(40));
            ui->ipv6_lb_LLA->setText(QString(info.LinkLocalAddress).left(40));
        }
        break;
    case DeviceContrl::CMD_IPv4_Set:
    case DeviceContrl::CMD_IPv6_Set:
    case DeviceContrl::CMD_DEVICE_status:
    default:
        break;
    }
}

void TabSetting::on_btn_apply_ws_clicked()
{
    if (device_manager->get_passwd_confirmed() || setting_confirmPasswd()){
        //setting data then apply
        cmdst_wifi_get wifi_para = device_manager->wifi_get_para();
        QString ssid;
        QString passwd;
        int encryptionType;
        if(ui->radioButton_searchWifi->isChecked()){
            ssid = ui->cb_ssid->currentText();
            int index = ui->cb_ssid->currentIndex();
            encryptionType = wifi_aplist_encryptionType[index < 0 ?0 :index];
        }else{
            encryptionType = ui->cb_encryptionType->currentIndex();
            ssid = ui->le_ssid->text();
        }
        if(1 == encryptionType){//wep key
            passwd = ui->le_wepkey->text();
        }else{
            //not use when encryptionType=0
            passwd = ui->le_passphrase->text();
        }
        //ssid
//        device_manager->wifi_set_ssid(&wifi_para ,wifi_ssid.toLatin1());
        device_manager->wifi_set_ssid(&wifi_para ,ssid.toLatin1());
        //passwd
//        device_manager->wifi_set_password(&wifi_para ,wifi_password.toLatin1());
        device_manager->wifi_set_password(&wifi_para ,passwd.toLatin1());
        //encryption type
//        wifi_para.encryption = wifi_encryptionType > 1 ? wifi_encryptionType + 1 :wifi_encryptionType;
        wifi_para.encryption = encryptionType > 1 ? encryptionType + 1 :encryptionType;
        //wep key id
//        wifi_para.wepKeyId = wifi_wepIndex + 1;
        wifi_para.wepKeyId = ui->cb_keyIndex->currentIndex() + 1;
        qLog(QString().sprintf("wep key ID to FW:%d" ,wifi_para.wepKeyId));
        //wifi enable
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
        QString passwd = DialogLogin::getPasswd(this ,&ok);
        if (ok && !passwd.isEmpty()){
            device_manager->passwd_set(passwd.toLatin1());
        }else{
            ok = false;
        }
        return ok;
}

void TabSetting::on_ipa_rb_ipv4_toggled(bool checked)
{
    if(checked){
        device_manager->emit_cmd_plus(DeviceContrl::CMD_IPv4_Get);
    }else{
        device_manager->emit_cmd_plus(DeviceContrl::CMD_IPv6_Get);
    }
}

void TabSetting::on_ip_btn_apply_clicked()
{
    if(!QString::compare(ui->le_newPassword->text() ,ui->le_confirmPassword->text())){
        if (setting_confirmPasswd()){
            if(ui->ipa_rb_ipv4->isChecked()){
                net_info_st info = device_manager->net_getIpv4info();
                info.IPMode = ui->ipa_rb_autoGet->isChecked()?3:4;
                info.IPAddress[0] = ui->ipa_sb_ipAddress->value();
                info.IPAddress[1] = ui->ipa_sb_ipAddress_2->value();
                info.IPAddress[2] = ui->ipa_sb_ipAddress_3->value();
                info.IPAddress[3] = ui->ipa_sb_ipAddress_4->value();
                info.SubnetMask[0] = ui->ipa_sb_subnetMask->value();
                info.SubnetMask[1] = ui->ipa_sb_subnetMask_2->value();
                info.SubnetMask[2] = ui->ipa_sb_subnetMask_3->value();
                info.SubnetMask[3] = ui->ipa_sb_subnetMask_4->value();
                info.GatewayAddress[0] = ui->ipa_sb_gateway->value();
                info.GatewayAddress[1] = ui->ipa_sb_gateway_2->value();
                info.GatewayAddress[2] = ui->ipa_sb_gateway_3->value();
                info.GatewayAddress[3] = ui->ipa_sb_gateway_4->value();
                device_manager->net_setIpv4info(&info);
                device_manager->emit_cmd_plus(DeviceContrl::CMD_IPv4_Set);
            }else{
                net_ipv6_st info = device_manager->net_getIpv6info();
                info.UseManualAddress = ui->ipv6_cb_UMA->isChecked() ?1 :0;
                info.DHCPv6 = ui->ipv6_cb_dhcp->isChecked() ?1 :0;
                memset(info.ManualAddress ,0 ,40);
                strcpy(info.ManualAddress ,ui->ipv6_le_manualAddress->text().toLatin1());
                info.ManualMask = ui->ipv6_sb_manualAddressMask->value();
                memset(info.IPv6ManualGatewayAddress ,0 ,40);
                strcpy(info.IPv6ManualGatewayAddress ,ui->ipv6_le_manualGatewayAddress->text().toLatin1());
                device_manager->net_setIpv6info(&info);
                device_manager->emit_cmd_plus(DeviceContrl::CMD_IPv6_Set);
            }
        }
    }
}
