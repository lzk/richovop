#include "tababout.h"
#include "ui_tababout.h"

TabAbout::TabAbout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabAbout)
{
    ui->setupUi(this);

    ui->label->installEventFilter(this);
}

TabAbout::~TabAbout()
{
    delete ui;
}

#include <QMouseEvent>
bool TabAbout::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();
    switch(type){
    case QEvent::MouseButtonPress:
        if (obj == ui->label){
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if(Qt::LeftButton == mouseEvent->button()
                    && mouseEvent->x() > 130 && mouseEvent->y() > 280
                    && mouseEvent->x() < 180 && mouseEvent->y() < 330
                    )        {
                slots_about_update();
            }
//            return true;
        }
        break;
    default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

#include <QDesktopServices>
#include<QUrl>
void TabAbout::slots_about_update()
{
    if(!QDesktopServices::openUrl(QUrl("http://www.lenovo.com"))){
    }
}
