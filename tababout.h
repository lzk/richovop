#ifndef TABABOUT_H
#define TABABOUT_H

#include <QWidget>

namespace Ui {
class TabAbout;
}

class TabAbout : public QWidget
{
    Q_OBJECT

public:
    explicit TabAbout(QWidget *parent = 0);
    ~TabAbout();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::TabAbout *ui;

    void slots_about_update();
};

#endif // TABABOUT_H
