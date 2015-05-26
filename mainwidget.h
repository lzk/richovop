#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

namespace Ui {
class MainWidget;
class TabCopy;
class TabSetting;
class TabAbout;
}
class NewWidget;

//class QProcess;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    Ui::TabCopy *tc;
//    NewWidget* tc;
    Ui::TabSetting *ts;
    Ui::TabAbout *ta;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
//    QProcess* ps;
    QAction* action_update;

    void initializeUi();
    void retranslateUi();
    void createActions();

private slots:
    void slots_update();
};

#endif // MAINWIDGET_H
