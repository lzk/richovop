#ifndef LOGO_ICON_H
#define LOGO_ICON_H

#include <QWidget>

namespace Ui {
class Logo_icon;
}

class Logo_icon : public QWidget
{
    Q_OBJECT

public:
    explicit Logo_icon(QWidget *parent = 0);
    ~Logo_icon();

private:
    Ui::Logo_icon *ui;
};

#endif // LOGO_ICON_H
