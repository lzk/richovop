#ifndef TABSETTING_H
#define TABSETTING_H

#include <QWidget>

namespace Ui {
class TabSetting;
}

class TabSetting : public QWidget
{
    Q_OBJECT

public:
    explicit TabSetting(QWidget *parent = 0);
    ~TabSetting();

private:
    Ui::TabSetting *ui;
};

#endif // TABSETTING_H
