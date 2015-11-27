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
        sl.removeFirst();
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
    int docSize =pCopyPara->orgSize;
    if(docSize == 3)    docSize = 0;
    else if(docSize != 4) docSize ++;
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
        if(0 == value)//letter
            pCopyPara->orgSize = 3;
        else if(4 != value)
            pCopyPara->orgSize = value - 1;
        else
            pCopyPara->orgSize = value;
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
            pCopyPara->paperSize = 1;//only A4
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
        if(!err){
            copy_data->this_copy = true;
            if(IsIDCardCopyMode(pCopyPara))
                copy_data->idCard_mode = true;
        }
        break;
    default:
        break;
    }
}

void TabCopy::cmdResult_getDeviceStatus(int err)
{
    if(copy_data->this_copy){
        switch(err){
        case STATUS_ready:
        case STATUS_sleep:
        case STATUS_TonerEnd:
            copy_data->status = true;
            copy_data->this_copy = false;
            if(copy_data->idCard_mode){
                copy_data->idCard_mode = false;
                if(IsIDCardCopyMode(pCopyPara))
                    on_IDCardCopy_clicked();
            }
            break;
        default:
            copy_data->status = false;
            break;
        }
        if(STATUS_CopyScanNextPage == err){
            if(copy_data->idCard_mode){
                main_widget->messagebox_show(tr("IDS_MSG_TurnCardOver"));
            }else{
                main_widget->messagebox_show(tr("IDS_MSG_PlaceNextPage"));
            }
        }else{
            main_widget->messagebox_hide();
        }
    }else{
        switch(err){
        case STATUS_ready:
        case STATUS_sleep:
        case STATUS_busy_printing:
        case STATUS_busy_scanningOrCoping:
        case STATUS_jam:
        case STATUS_CopyScanNextPage:
        case STATUS_TonerEnd:
        case STATUS_other:
            copy_data->status = true;
            break;
        default:
            copy_data->status = false;
            break;
        }
        copy_data->idCard_mode = false;
        main_widget->messagebox_hide();
    }

//    switch(err){
//    case STATUS_ready:
//    case STATUS_sleep:
//    case STATUS_TonerEnd:
//        copy_data->status = true;
//        copy_data->this_copy = false;
//        if(copy_data->idCard_mode){
//            copy_data->idCard_mode = false;
//            if(IsIDCardCopyMode(pCopyPara))
//                on_IDCardCopy_clicked();
//        }
//        break;
//    default:
//        copy_data->status = false;
//        break;
//    }
//    if(copy_data->this_copy && STATUS_CopyScanNextPage == err){
//        if(copy_data->idCard_mode){
//            main_widget->messagebox_show(tr("IDS_MSG_TurnCardOver"));
//        }else{
//            main_widget->messagebox_show(tr("IDS_MSG_PlaceNextPage"));
//        }
//    }else{
//        main_widget->messagebox_hide();
//    }
    ui->copy->setEnabled(copy_data->status);
}

void TabCopy::on_copy_clicked()
{
    copy_data->status = false;
    ui->copy->setEnabled(copy_data->status);
    device_manager->copy_set_para(pCopyPara);
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
        pCopyPara->paperSize = 1;//A4
//        if(3 == pCopyPara->paperSize || 5 == pCopyPara->paperSize){
//            pCopyPara->paperSize = 0;
//        }
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
