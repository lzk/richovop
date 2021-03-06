#include "tabcopy.h"
#include "ui_tabcopy.h"

#include "mainwidget.h"
#include "app/log.h"
#include "app/devicecontrol.h"
#include "app/deviceapp.h"
#include "scalingsettingkeyboard.h"
#include "copiessettingkeyboard.h"
#include "app/devicemanager.h"
#include "app/devicedata.h"

//#include <QSettings>
extern bool system_paper_is_A4();
extern bool g_region_paper_is_A4;
bool have_got_region_from_FW = false;

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

static const int uiToOrgSize[] =
//{3 ,0 ,1 ,6 ,2 ,5 ,4 ,7};
{0 ,1 ,2 ,3 ,4 ,6 ,5 ,7};
static const int orgSizeToUi[] =
//{1 ,2 ,4 ,0 ,6 ,5 ,3 ,7};// A4 A5 B5 Letter Executive
{0 ,1 ,2 ,3 ,4 ,6 ,5 ,7};
static const int orgSizeToOutputSize[] =
{1 ,2 ,4 ,0 ,6 ,5 ,3 ,7};
static const int output_size[][2] =
{
  {216, 279},//Letter 0
    {210,297},//A4 1
    {148,210},//A5 2
    {105,148},//A6 3
    {182,257},//B5 4
    {128 ,182},//B6 5
    {184 ,267},//Executive 6
    {185 ,260}//16K 7
};

#define _GetSizeScaling(ds ,os ,index)  ((output_size[os][index] - 8.2) / (output_size[orgSizeToOutputSize[ds]][index] - 8.2) + 0.005)
#define GetSizeScaling(ds ,os ,scaling) \
{ \
    double  scaling_width = _GetSizeScaling(ds ,os ,0);  \
    double scaling_height = _GetSizeScaling(ds ,os ,1); \
    scaling = 100 * (scaling_width < scaling_height ? scaling_width :scaling_height); \
}

TabCopy::TabCopy(MainWidget* widget,DeviceManager* dm ,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabCopy),
    main_widget(widget),
    device_manager(dm)
{
    copy_data = dm->device_data->get_copy_data();
    ui->setupUi(this);

    pCopyPara = &copyPara;
    int i;
//    for(i = 0 ;i < sizeof(output_size_list) / sizeof(output_size_list[0]) ;i++){
    for(i = 0 ;i < ui->combo_outputSize->count() ;i++){
//        stringlist_output_size << QApplication::translate("TabCopy" ,output_size_list[i]);
        stringlist_output_size << ui->combo_outputSize->itemText(i);
    }

    connect(ui->scaling_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(ui->scaling_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(ui->copies_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(ui->copies_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(ui->density_minus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
    connect(ui->density_plus ,SIGNAL(clicked()) ,this ,SLOT(slots_copy_pushbutton()));
//    connect(ui->text ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_copy_pushbutton()));
//    connect(ui->photo ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_copy_radio(bool)));
//    connect(ui->combo_documentType ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(ui->combo_documentSize ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(ui->combo_outputSize ,SIGNAL(activated(int)) ,this ,SLOT(slots_copy_combo(int)));
//    connect(ui->combo_nIn1Copy ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
//    connect(ui->combo_dpi ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));

//    action_copy_default = new QAction(this);
//    connect(action_copy_default ,SIGNAL(triggered()) ,this ,SLOT(slots_copy_default()));
//    connect(ui->btn_default ,SIGNAL(clicked()) ,action_copy_default ,SLOT(trigger()));

    ui->combo_documentType->installEventFilter(this);
    ui->combo_documentSize->installEventFilter(this);
    ui->combo_outputSize->installEventFilter(this);
    ui->combo_nIn1Copy->installEventFilter(this);
    ui->combo_dpi->installEventFilter(this);

//    keyboard_scaling = new ScalingSettingKeyboard;
    keyboard_scaling = new ScalingSettingKeyboard(this);
    keyboard_scaling->hide();
    ui->scaling->installEventFilter(this);
    connect(keyboard_scaling ,SIGNAL(sendScalingData(QString)) ,this ,SLOT(slots_copy_keyboard(QString)));

//    keyboard_copies = new CopiesSettingKeyboard;
    keyboard_copies = new CopiesSettingKeyboard(this);
    keyboard_copies->hide();
    ui->copies->installEventFilter(this);
    connect(keyboard_copies ,SIGNAL(sendCopiesData(QString)) ,this ,SLOT(slots_copy_keyboard(QString)));

    on_btn_default_clicked();
}

TabCopy::~TabCopy()
{
    delete ui;
}

bool TabCopy::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();
    switch(type){
    case QEvent::Wheel:
        if(qobject_cast<QComboBox*>(obj))
            return true;
        break;
    case QEvent::MouseButtonPress:
        if(obj == ui->copies){
            if(!keyboard_copies->isVisible()){
                keyboard_copies->set_num(ui->copies->text().toInt());
                keyboard_copies->exec();
            }
        }else if(obj == ui->scaling){
            if(!keyboard_scaling->isVisible() && ui->label_scaling->isEnabled()){
                keyboard_scaling->set_num(ui->scaling->text().remove(QChar('%')).toInt());
                keyboard_scaling->exec();
            }
        }
        break;
default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

#define SetWhite(widget) widget->setStyleSheet("background-color:white")
#define SetGray(widget) widget->setStyleSheet("background-color:gray")
#define SetIDCardCopy(widget) widget->setStyleSheet(":enabled:!pressed{background-color:#faaf40}")
#define SetCopy(widget) widget->setStyleSheet(":enabled:!pressed{background-color:#71c2cd}")
#define IsIDCardCopyMode(pCopyPara) (pCopyPara->nUp == 4)
#define SetIDCardCopyMode(pCopyPara) (pCopyPara->nUp = 4)
void TabCopy::updateCopy()
{
//    copycmdset copyPara = device_manager->copy_get_para();
//    copycmdset* pCopyPara = &copyPara;
    disconnect(ui->photo ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_copy_radio(bool)));
    disconnect(ui->combo_documentType ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
//    disconnect(ui->combo_documentSize ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
//    disconnect(ui->combo_outputSize ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
    disconnect(ui->combo_nIn1Copy ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
    disconnect(ui->combo_dpi ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
    //scaling ui
    ui->scaling->setText(QString("%1%").arg(pCopyPara->scale));
    //copies ui
    ui->copies->setText(QString("%1").arg(pCopyPara->copyNum));
    //density ui
//    SetGray(ui->density1);
    switch(pCopyPara->Density)
    {
    case 1:SetWhite(ui->density2);SetWhite(ui->density3);SetWhite(ui->density4);SetWhite(ui->density5);break;
    case 2:SetGray(ui->density2);SetWhite(ui->density3);SetWhite(ui->density4);SetWhite(ui->density5);break;
    case 3:SetGray(ui->density2);SetGray(ui->density3);SetWhite(ui->density4);SetWhite(ui->density5);break;
    case 4:SetGray(ui->density2);SetGray(ui->density3);SetGray(ui->density4);SetWhite(ui->density5);break;
    default:SetGray(ui->density2);SetGray(ui->density3);SetGray(ui->density4);SetGray(ui->density5);break;
    }
    //scanning mode ui
    pCopyPara->scanMode ?ui->text->setChecked(true):ui->photo->setChecked(true);

    //output size ui
    int value = pCopyPara->paperSize;

    QStringList sl(stringlist_output_size);
    if(IsIDCardCopyMode(pCopyPara)){
        if(g_region_paper_is_A4)
            sl.removeFirst();//only A4
        else
            sl.removeLast();//only letter
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        sl.removeLast();
        value = 0;
    }else if(1 == pCopyPara->nUp){//2in1 hide a6 b6
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
    ui->combo_outputSize->clear();
    ui->combo_outputSize->insertItems(0 ,sl);
    ui->combo_outputSize->setCurrentIndex(value);
    //n in 1 copy ui
    if(!IsIDCardCopyMode(pCopyPara)){
        value =  pCopyPara->paperSize;
        ui->combo_nIn1Copy->clear();
        if(3 == value || 5 == value){
            ui->combo_nIn1Copy->addItem("1");
        }else if(value > 1){
            ui->combo_nIn1Copy->addItem("1");
            ui->combo_nIn1Copy->addItem("2");
        }else{
            ui->combo_nIn1Copy->addItem("1");
            ui->combo_nIn1Copy->addItem("2");
            ui->combo_nIn1Copy->addItem("4");
            ui->combo_nIn1Copy->addItem("9");
        }
        ui->combo_nIn1Copy->setCurrentIndex(pCopyPara->nUp);
    }else{
        ui->combo_nIn1Copy->setCurrentIndex(0);
    }
    ui->combo_documentType->setCurrentIndex(pCopyPara->mediaType);
    int docSize = pCopyPara->orgSize <= 7 ?pCopyPara->orgSize :0;
    docSize = orgSizeToUi[docSize];
    ui->combo_documentSize->setCurrentIndex(docSize);
    ui->combo_dpi->setCurrentIndex(pCopyPara->dpi);

    //check valid
    if(pCopyPara->scale >= 400) {pCopyPara->scale = 400; ui->scaling_plus->setEnabled(false);}
    else{ui->scaling_plus->setEnabled(true);}
    if(pCopyPara->scale <= 25) {pCopyPara->scale = 25; ui->scaling_minus->setEnabled(false);}
    else{ui->scaling_minus->setEnabled(true);}

    if(pCopyPara->copyNum >= 99) {pCopyPara->copyNum = 99; ui->copies_plus->setEnabled(false);}
    else{ui->copies_plus->setEnabled(true);}
    if(pCopyPara->copyNum <= 1) {pCopyPara->copyNum = 1; ui->copies_minus->setEnabled(false);}
    else{ui->copies_minus->setEnabled(true);}

    if(pCopyPara->Density >= 5) {pCopyPara->Density = 5; ui->density_plus->setEnabled(false);}
    else{ui->density_plus->setEnabled(true);}
    if(pCopyPara->Density <= 1) {pCopyPara->Density = 1; ui->density_minus->setEnabled(false);}
    else{ui->density_minus->setEnabled(true);}

    if(IsIDCardCopyMode(pCopyPara))
    {
        ui->combo_documentSize->setEnabled(false);
        ui->combo_nIn1Copy->setEnabled(false);
        ui->combo_dpi->setEnabled(false);
        SetIDCardCopy(ui->copy);
        ui->bg_scaling->setEnabled(false);
        ui->IDCardCopy->setEnabled(true);
//        ui->combo_outputSize->setEnabled(false);
    }
    else
    {
//        ui->combo_outputSize->setEnabled(true);
        ui->combo_documentSize->setEnabled(true);
        ui->combo_nIn1Copy->setEnabled(true);
        ui->combo_dpi->setEnabled(true);
        SetCopy(ui->copy);
        if(pCopyPara->nUp)    {
            ui->bg_scaling->setEnabled(false);
            ui->IDCardCopy->setEnabled(false);
        }else{
            ui->bg_scaling->setEnabled(true);
            ui->IDCardCopy->setEnabled(true);
        }
    }
    ui->copy->setEnabled(copy_data->status);
    connect(ui->photo ,SIGNAL(toggled(bool)) ,this ,SLOT(slots_copy_radio(bool)));
    connect(ui->combo_documentType ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
//    connect(ui->combo_documentSize ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
//    connect(ui->combo_outputSize ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(ui->combo_nIn1Copy ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
    connect(ui->combo_dpi ,SIGNAL(currentIndexChanged(int)) ,this ,SLOT(slots_copy_combo(int)));
}

void TabCopy::slots_copy_combo(int value)
{
    QObject* sd = sender();
    if(sd == ui->combo_documentType)
        pCopyPara->mediaType = value;
    else if(sd == ui->combo_dpi)
        pCopyPara->dpi = value;
    else if(sd == ui->combo_documentSize) {
       pCopyPara->orgSize =  uiToOrgSize[value];
//        if(!pCopyPara->nUp)
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }else if(sd == ui->combo_nIn1Copy){//disable when IsIDCardCopyMode
        pCopyPara->nUp = value;
//        if(value)        {
//            pCopyPara->scale = 100;
//        }else{
//                GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
//        }
        GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }    else if(sd == ui->combo_outputSize){
        if(IsIDCardCopyMode(pCopyPara)){
            if(g_region_paper_is_A4)
                pCopyPara->paperSize = 1;//only A4
            else
                pCopyPara->paperSize = 0;//only letter
        }else{
            if(1 == pCopyPara->nUp){
                if(value > 3)                value += 2;
                else if(value > 2)        value ++;
            }
            pCopyPara->paperSize = value;
//           if(!pCopyPara->nUp)
            GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
        }
    }
    updateCopy();
}

void TabCopy::slots_copy_pushbutton()
{
    QObject* sd = sender();
    if(sd == ui->scaling_minus){
        pCopyPara->scale --;
    }else if(sd == ui->scaling_plus){
        pCopyPara->scale ++;
    }else if(sd == ui->density_minus){
        pCopyPara->Density --;
    }else if(sd == ui->density_plus){
        pCopyPara->Density ++;
    }else if(sd == ui->copies_minus){
        pCopyPara->copyNum --;
    }else if(sd == ui->copies_plus){
        pCopyPara->copyNum ++;
    }
    updateCopy();
}

void TabCopy::slots_copy_radio(bool checked)
{
    QObject* sd = sender();
    if(sd == ui->photo){
            pCopyPara->scanMode = !checked;
            updateCopy();
    }
}

void TabCopy::slots_copy_keyboard(QString str)
{
    QObject* sd = sender();
    if(sd == keyboard_copies){
        pCopyPara->copyNum = str.toInt();
    }else if(sd == keyboard_scaling){
        pCopyPara->scale = str.toInt();
    }
    updateCopy();
}

void TabCopy::slots_cmd_result(int cmd ,int err)
{
    switch(cmd)
    {
    case DeviceContrl::CMD_DEVICE_status:
        cmdResult_getDeviceStatus(err);
        break;
    case DeviceContrl::CMD_COPY:
        if(cmd_err_handler(err)){
            copy_data->this_copy = true;
        }
        break;
    case DeviceContrl::CMD_PRN_GetRegion:{
        //set default region
//        QSettings settings(QApplication::applicationDirPath() +"/settings.conf",QSettings::NativeFormat);
        if(!err){
            if(!have_got_region_from_FW){
                have_got_region_from_FW = true;
            }
            _Q_LOG("get region from FW success");
            cmdst_region region = device_manager->printer_getRegion();
            g_region_paper_is_A4 = (1 != region);//North America
            device_manager->copy_update_defaultPara();
            updateCopy();
//            settings.beginGroup("copy");
//            settings.setValue("default paper size" ,g_region_paper_is_A4 ?1 :0);
//            settings.endGroup();
        }else{
//            settings.beginGroup("copy");
//            int setting_paper_size = settings.value("default paper size" ,2).toInt();
//            settings.endGroup();
//            if(2 == setting_paper_size){
//                g_region_paper_is_A4 = system_paper_is_A4();
//            }else{
//                g_region_paper_is_A4 = setting_paper_size;
//            }
            _Q_LOG("get region from FW fail");
            if(!have_got_region_from_FW){
                _Q_LOG("get system local region");
                g_region_paper_is_A4 = system_paper_is_A4();
                device_manager->copy_update_defaultPara();
                updateCopy();
            }
        }
        static bool b_init = true;
        if(b_init){
            b_init = false;
            on_btn_default_clicked();
        }
        break;
    }
    default:
        break;
    }
}

bool TabCopy::cmd_err_handler(int err)
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
    case STATUS_TonerEnd:
        main_widget->messagebox_exec(tr("ResStr_Toner_End") + "\n" + tr("ResStr_Please_Replace_Toner"));
        break;
    default:
        break;
    }
    return !err;
}

void TabCopy::cmdResult_getDeviceStatus(int err)
{
    if(copy_data->this_copy){
        switch(err){
        case STATUS_ready:
        case STATUS_sleep:
        case STATUS_TonerNearEnd:
        case STATUS_TonerEnd:
            copy_data->this_copy = false;
                if(IsIDCardCopyMode(pCopyPara))
                    on_IDCardCopy_clicked();
            copy_data->status = true;
            main_widget->messagebox_hide();
            break;
        case STATUS_CopyScanNextPage:
            copy_data->status = false;
            main_widget->messagebox_show(tr("IDS_MSG_PlaceNextPage"));
            break;
        case STATUS_IDCardCopyTurnCardOver:
            copy_data->status = false;
            main_widget->messagebox_show(tr("IDS_MSG_TurnCardOver"));
            break;
        default:
            copy_data->status = false;
            main_widget->messagebox_hide();
            break;
        }
    }else{
        switch(err){
        case STATUS_ready:
        case STATUS_sleep:
        case STATUS_TonerNearEnd:
        case STATUS_TonerEnd:
//        case STATUS_busy_printing:
//        case STATUS_busy_scanningOrCoping:
//        case STATUS_jam:
//        case STATUS_CopyScanNextPage:
//        case STATUS_IDCardCopyTurnCardOver:
//        case STATUS_other:
//        case ERR_printer_have_jobs:
//        case ERR_sane_scanning:
            copy_data->status = true;
            break;
        default:
            copy_data->status = false;
            break;
        }
        main_widget->messagebox_hide();
    }

    ui->copy->setEnabled(copy_data->status);
}

void TabCopy::on_copy_clicked()
{
    copy_data->status = false;
    ui->copy->setEnabled(copy_data->status);
    device_manager->copy_set_para(pCopyPara);
   C_LOG("copy set scale:%d\torgSize:%d" ,pCopyPara->scale ,pCopyPara->orgSize);
    device_manager->emit_cmd_plus(DeviceContrl::CMD_COPY);
}

void TabCopy::on_IDCardCopy_clicked()
{
    if(IsIDCardCopyMode(pCopyPara)){
        pCopyPara->nUp = 0;
        GetSizeScaling(pCopyPara->orgSize ,pCopyPara->paperSize ,pCopyPara->scale);
    }else{
        SetIDCardCopyMode(pCopyPara);
        pCopyPara->dpi = 1;//600 * 600
        pCopyPara->scale = 100;
        if(g_region_paper_is_A4)
            pCopyPara->paperSize = 1;//only A4
        else
            pCopyPara->paperSize = 0;//only letter
    }
    updateCopy();
}

void TabCopy::on_btn_default_clicked()
{
    device_manager->copy_set_defaultPara(pCopyPara);
    updateCopy();
}

void TabCopy::set_copy_data(struct CopyData* cd)
{
    copy_data = cd;
}
