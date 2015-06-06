#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

namespace Ui {
class MainWidget;
class TabCopy;
class TabSetting;
class TabAbout;
}
//class QProcess;
class DeviceManager;
class QStringList;
#include<QThread>
#include <QTimer>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    Ui::TabCopy *tc;
    Ui::TabSetting *ts;
    Ui::TabAbout *ta;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
//    QAction* action_refresh;
    DeviceManager* deviceManager;
    QThread deviceManageThread;
    QTimer timer;
    int status;

    void initializeUi();
    void retranslateUi();
    void createActions();
    void updateUi();
    void initialize();

signals:
   void  signals_device_status();
   void signals_copy();

public slots:
    void slots_device_status(int);

private:
    ///////////////////////////tab about//////////////////////
//    QAction* action_about_update;
    void initializeTabAbout();

    ///////////////////////////tab copy///////////////////
    //    QAction* action_copy_default;
    QStringList stringlist_output_size;
    void initializeTabCopy();
    void updateCopy();

    //////////////////////////tab setting///////////////////
    void initializeTabSetting();
    void validateSsidPassword(const QString& ,const QString&);

private slots:
    //////////////////tab about///////////////////////////
    void slots_about_update();
    /////////////////tab copy///////////////////////////////
    void slots_copy_minus_plus();
    void slots_copy_scaningMode();
    void slots_copy_combo(int);
    void slots_copy_default();

private slots:
    void on_refresh_clicked();
    void on_comboBox_deviceList_activated(int index);

    //////////////////////////tab setting///////////////////
    void slots_setting_radiobutton(bool);
    void slots_setting_lineedit_textChanged(const QString &arg1);
};

#endif // MAINWIDGET_H
