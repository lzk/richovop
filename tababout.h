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

    void watched_poptime();
    bool get_poptime_checked();

private slots:
    void on_checkBox_toggled(bool checked);

private:
    Ui::TabAbout *ui;
};

#endif // TABABOUT_H
