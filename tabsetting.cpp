#include "tabsetting.h"
#include "ui_tabsetting.h"

#include "app/log.h"
#include "app/devicecontrol.h"
#include "app/deviceapp.h"
#include "app/devicemanager.h"

#include <QInputDialog>
#include "dialoglogin.h"
#include "mainwidget.h"

TabSetting::TabSetting(MainWidget* widget,DeviceManager* dm ,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabSetting),
    main_widget(widget),
    device_manager(dm),
    disable_emit(false)
{
    ui->setupUi(this);

    init_wifi();

    //init power save
    ui->checkBox_powerOff->hide();

    init_ip();
#ifndef FUTURE_SUPPORT
    ui->listWidget->item(2)->setHidden(true);
    ui->listWidget->item(3)->setHidden(true);
    ui->listWidget->item(4)->setHidden(true);
#endif

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
//    ui->cb_ssid->setView(new QListView);
//    ui->cb_ssid->view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(ui->radioButton_searchWifi ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_radiobutton(bool)));
    connect(ui->le_ssid,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->le_wepkey,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->le_passphrase,SIGNAL(textChanged(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->cb_encryptionType,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->cb_keyIndex,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
    connect(ui->cb_ssid,SIGNAL(activated(QString)) ,this ,SLOT(slots_wifi_textChanged(QString)));
//    connect(ui->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_wifi_checkbox(bool)));


    ui->cb_encryptionType->installEventFilter(this);
    ui->cb_keyIndex->installEventFilter(this);
    ui->cb_ssid->installEventFilter(this);
    ui->le_ssid->installEventFilter(this);

    wifi_update();
    ui->checkBox->hide();//checkbox hide
}

void TabSetting::init_ip()
{
    ui->ipv6_sb_manualAddressMask->setMinimum(0x80000000);
    ui->ipv6_sb_manualAddressMask->setMaximum(0x7fffffff);
    ui->page5Widget->installEventFilter(this);
}

bool TabSetting::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();
    switch(type){
    case QEvent::Wheel:
        if(qobject_cast<QComboBox*>(obj))
            return true;
        break;
    case QEvent::FocusOut:
        if(obj == ui->le_ssid){
            ui->le_ssid->home(false);
        }
        break;
    case QEvent::Show:
        if(obj == ui->le_ssid){
            if(!ui->le_ssid->hasFocus()){
                ui->le_ssid->home(false);
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

#include <QDebug>
void TabSetting::slots_wifi_textChanged(const QString &arg1)
{    
    if(disable_emit)
        return;
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
    wifi_ssid = QString(wifi_para.ssid).left(32);
    //ssid aplist
    _Q_LOG("");
    _Q_LOG("get aplist from FW");
    int current_ssid = -1;
    int encryption;
    QString ssid;
    QStringList ssid_list;
    for(int i = 0 ;i < NUM_OF_APLIST ;i++){
        ssid = QString(aplist.aplist[i].ssid).left(32);
        if(ssid.isEmpty()){
            break;
        }else{
            ssid_list << ssid;
            encryption = aplist.aplist[i].encryption & 7;
            if(encryption > 4) encryption = 4;
            wifi_aplist_encryptionType[i] = encryption > 1 ?encryption -1 : encryption;
            if(0x80 == (0x80 & aplist.aplist[i].encryption)){//FW LShell spec 0811
//                _Q_LOG("use bit 7 and after FW LShell spec 0811");
                if(!ssid.compare(wifi_ssid))
                    current_ssid = i;
            }
            _Q_LOG(QString().sprintf("index:%d encryptionType:%#.2x ssid:" ,i ,aplist.aplist[i].encryption) + ssid);
        }
    }
/*    if(-1 == current_ssid){
        for(int i = 0 ;i < NUM_OF_APLIST ;i++){
            QString ssid = QString(aplist.aplist[i].ssid).left(32);
            if(ssid.isEmpty()){
                break;
            }else{
                _Q_LOG("do not use bit 7 and before FW LShell spec 0811");
                if(!ssid.compare(wifi_ssid))
                    current_ssid = i;
            }
        }
    }//*/
    _Q_LOG("");
    if(-1 == current_ssid){
        _Q_LOG("do not connect any ssid");
    }
    _Q_LOG("wifi_para.ssid:" + wifi_ssid);
    C_LOG("wifi_para.encryption:%d" ,wifi_para.encryption);
    C_LOG("wifi_para.wepKeyId:%d" ,wifi_para.wepKeyId);
    encryption = wifi_para.encryption;
    if(encryption > 4) encryption = 4;
    wifi_encryptionType = encryption > 1 ?encryption -1 : encryption;
    if(wifi_para.wepKeyId){
        wifi_wepIndex = (wifi_para.wepKeyId - 1) % 4;
    }else{
        wifi_wepIndex = wifi_default_wepIndex;
    }
    disable_emit = true;
    ui->cb_ssid->clear();
    ui->cb_ssid->addItems(ssid_list);
    ui->cb_ssid->setCurrentIndex(current_ssid);
    ui->le_ssid->setText(wifi_ssid);
    ui->le_ssid->home(false);
    //encryption
    wifi_ms_encryptionType = wifi_encryptionType;
    wifi_sw_encryptionType = wifi_encryptionType;
     ui->cb_encryptionType->setCurrentIndex(wifi_encryptionType);
    //wep key index
     wifi_ms_wepIndex = wifi_wepIndex;
     wifi_sw_wepIndex = wifi_wepIndex;
     ui->cb_keyIndex->setCurrentIndex(wifi_wepIndex);
    //clear passwd
    wifi_sw_password.clear();//Qt4
    ui->le_passphrase->clear();
    ui->le_wepkey->clear();
    wifi_password.clear();
    if(ui->le_passphrase->isVisible()){
        ui->le_passphrase->setFocus();
    }else if(ui->le_wepkey->isVisible()){
        ui->le_wepkey->setFocus();
    }

    //update ui else
    wifi_update(wifi_ssid ,wifi_password ,wifi_encryptionType);
    disable_emit = false;
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

bool TabSetting::cmd_err_handler(int err)
{
    switch(err){
    case STATUS_busy_printing:
        main_widget->messagebox_exec(tr("IDS_MSG_Printering"));
        break;
    case STATUS_busy_scanningOrCoping:
    case STATUS_CopyScanNextPage:
    case STATUS_IDCardCopyTurnCardOver:
        main_widget->messagebox_exec(tr("IDS_MSG_MachineBusy"));
        break;
    case STATUS_jam:
        main_widget->messagebox_exec(tr("IDS_ERR_JAM"));
        break;
    default:
        break;
    }
    return !err;
}

void TabSetting::slots_cmd_result(int cmd ,int err)
{
    //handle cmd result
    switch(cmd)
    {
    case DeviceContrl::CMD_PRN_TonerEnd_Get:
        if(cmd_err_handler(err)){
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
        if(cmd_err_handler(err)){
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
        if(cmd_err_handler(err)){
//            ui->le_confirmPassword->clear();
//            ui->le_newPassword->clear();
        }else if(ERR_Password_incorrect == err){//password incorrect
            on_btn_apply_mp_clicked();
        }
        break;
    case DeviceContrl::CMD_WIFI_refresh_plus:
        if(cmd_err_handler(err))
            result_wifi_getAplist();
        break;
    case DeviceContrl::CMD_PRN_PowerSave_Get:
        if(cmd_err_handler(err)){
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
        if(cmd_err_handler(err)){
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
        if(cmd_err_handler(err)){
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
        device_manager->wifi_set_ssid(&wifi_para ,ssid.toLatin1());
        _Q_LOG("wifi apply: wifi_para.ssid:" + ssid);
        //passwd
        device_manager->wifi_set_password(&wifi_para ,passwd.toLatin1());
        //encryption type
        wifi_para.encryption = encryptionType > 1 ? encryptionType + 1 :encryptionType;
        C_LOG("wifi apply: wifi_para.encryption:%d" ,wifi_para.encryption);
        //wep key id
//        wifi_para.wepKeyId = wifi_wepIndex + 1;
        wifi_para.wepKeyId = ui->cb_keyIndex->currentIndex() + 1;
        C_LOG("wifi apply: wifi_para.wepKeyId:%d" ,wifi_para.wepKeyId);
        //wifi enable
        if(!(wifi_para.wifiEnable & 1)){
            _Q_LOG("wifi apply:wifi have not been enable ,enable it");
            wifi_para.wifiEnable &= ~1;
            wifi_para.wifiEnable |= 1;//bit 0
//            wifi_para.wifiEnable |= ui->checkBox->isChecked() ? 1 : 0;//bit 0
        }

        device_manager->wifi_set_para(&wifi_para);
        device_manager->emit_cmd_plus(DeviceContrl::CMD_WIFI_apply_plus);
    }
}

void TabSetting::on_btn_apply_mp_clicked()
{
    if(!QString::compare(ui->le_newPassword->text() ,ui->le_confirmPassword->text())){
        if (device_manager->get_passwd_confirmed() ||setting_confirmPasswd()){
            device_manager->set_tmp_passwd(ui->le_newPassword->text().toLatin1());
            device_manager->emit_cmd_plus(DeviceContrl::CMD_PASSWD_set_plus);
        }
    }else{
        main_widget->messagebox_exec(tr("IDS_ST_PasswordsDifferent"));
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
                if(ui->ipv6_le_manualAddress->text().count() >= 40){
                    memcpy(info.ManualAddress ,ui->ipv6_le_manualAddress->text().toLatin1() ,40);
                }else{
                    memset(info.ManualAddress ,0 ,40);
                    strcpy(info.ManualAddress ,ui->ipv6_le_manualAddress->text().toLatin1());
                }
                info.ManualMask = ui->ipv6_sb_manualAddressMask->value();
                if(ui->ipv6_le_manualGatewayAddress->text().count() >= 40){
                    memcpy(info.IPv6ManualGatewayAddress ,ui->ipv6_le_manualGatewayAddress->text().toLatin1() ,40);
                }else{
                    memset(info.IPv6ManualGatewayAddress ,0 ,40);
                    strcpy(info.IPv6ManualGatewayAddress ,ui->ipv6_le_manualGatewayAddress->text().toLatin1());
                }
                device_manager->net_setIpv6info(&info);
                device_manager->emit_cmd_plus(DeviceContrl::CMD_IPv6_Set);
            }
        }
    }
}

void TabSetting::on_listWidget_currentRowChanged(int currentRow)
{
    switch(currentRow){
    case 0:
        on_btn_refresh_clicked();
        break;
    case 2:
        device_manager->emit_cmd_plus(DeviceContrl::CMD_PRN_TonerEnd_Get);
        break;
    case 3:
        device_manager->emit_cmd_plus(DeviceContrl::CMD_PRN_PowerSave_Get);
        break;
    case 4:
        if(ui->ipa_rb_ipv4->isChecked()){
            on_ipa_rb_ipv4_toggled(true);
        }else{
            ui->ipa_rb_ipv4->setChecked(true);
        }
        break;
    default:
         break;
    }
}

void TabSetting::on_le_newPassword_textEdited(const QString &arg1)
{
    QRegExp regexp;
    regexp.setPattern("^[0-9a-zA-Z]{0,32}$");
    if( -1 != regexp.indexIn(arg1)){
        last_newPassword = arg1;
        if(ui->le_newPassword->text().isEmpty() || ui->le_confirmPassword->text().isEmpty()){
            ui->btn_apply_mp->setEnabled(false);
        }else{
            ui->btn_apply_mp->setEnabled(true);
        }
    }else{
        ui->le_newPassword->setText(last_newPassword);
    }
}

void TabSetting::on_le_confirmPassword_textEdited(const QString &arg1)
{
    QRegExp regexp;
    regexp.setPattern("^[0-9a-zA-Z]{0,32}$");
    if( -1 != regexp.indexIn(arg1)){
        last_confirmPassword = arg1;
        if(ui->le_newPassword->text().isEmpty() || ui->le_confirmPassword->text().isEmpty()){
            ui->btn_apply_mp->setEnabled(false);
        }else{
            ui->btn_apply_mp->setEnabled(true);
        }
    }else{
        ui->le_confirmPassword->setText(last_confirmPassword);
    }
}
