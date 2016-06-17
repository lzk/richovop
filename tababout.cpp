#include "tababout.h"
#include "ui_tababout.h"
#define VERSION_DEFINE 1
#include "version.h"

#include <QSettings>
#include <QDateTime>
#include <QDebug>
TabAbout::TabAbout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabAbout)
{
    ui->setupUi(this);

    watched_poptime();
    ui->label->setText(
                QString().sprintf("<html><head/><body>"
                                  "<p><span style=\" font-size:18pt;\">%s</span></p>"
                                  "<p>%s</p>"
                                  "<p>%s</p>"
                                  "</body></html>"
                                  ,vop_name ,vop_version ,copy_right));
//    ui->label->installEventFilter(this);
}

TabAbout::~TabAbout()
{
    delete ui;
}

void TabAbout::on_checkBox_toggled(bool checked)
{
    QSettings settings;
//    QSettings settings(QApplication::applicationDirPath() +"/settings.conf",QSettings::NativeFormat);
    if(checked){
//        qDebug()<<"setting popup time"<<QTime::currentTime();
        settings.setValue("app/popup time" ,QDateTime::currentMSecsSinceEpoch() + 2592000000);//(1000*60*60*24*30));//30day
    }else{
        settings.setValue("app/popup time" ,0);
    }
}

void TabAbout::watched_poptime()
{
    QSettings settings;
//    QSettings settings(QApplication::applicationDirPath() +"/settings.conf",QSettings::NativeFormat);
    qint64 currenttime = settings.value("app/popup time").toLongLong();
    disconnect(ui->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(on_checkBox_toggled(bool)));
    if(currenttime == 0){
        ui->checkBox->setChecked(false);
    }else{
        if(currenttime > QDateTime::currentMSecsSinceEpoch()){
            ui->checkBox->setChecked(true);
        }else{
            ui->checkBox->setChecked(false);
            on_checkBox_toggled(false);
//            qDebug()<<"popup time coming!"<<QTime::currentTime();
        }
    }
    connect(ui->checkBox ,SIGNAL(toggled(bool)) ,this ,SLOT(on_checkBox_toggled(bool)));
}

bool TabAbout::get_poptime_checked()
{
    return ui->checkBox->isChecked();
}
