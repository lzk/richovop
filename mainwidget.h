/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QMessageBox>

class QProgressDialog;
class DeviceManager;
class CopiesSettingKeyboard;
class ScalingSettingKeyboard;

namespace Ui {
class MainWidget;
class TabCopy;
class TabSetting;
class TabAbout;
}

class MessageBox:public QMessageBox
{
protected:
    void showEvent(QShowEvent* event)
    {
        QWidget* label = findChild<QWidget*>("qt_msgbox_label");
        if(label){
//            label->setMinimumSize(400,300);
            label->setMinimumWidth(400);
        }
        QMessageBox::showEvent(event);
    }
};

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
    static QMessageBox::StandardButton showMessageBox(const QString &text,
          QMessageBox::StandardButtons buttons = QMessageBox::Ok,
         QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
         const QString &title = tr("<h3>Lenovo Virtual Panel</h3>"));

private:
    Ui::MainWidget *ui;
    Ui::TabCopy *tc;
    Ui::TabSetting *ts;
    Ui::TabAbout *ta;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
//    QAction* action_refresh;
    QTimer timer;
    QProgressDialog* progressDialog;
    bool device_status;
    DeviceManager* device_manager;

    void initializeUi();
    void retranslateUi();
    void createActions();
    void updateUi();
    void initialize();

    void emit_cmd(int);
signals:
   void signals_cmd(int);
   void signals_deviceChanged(const QString&);
   void signals_emit_cmd(int);

public slots:
   void slots_cmd_result(int ,int);
   void slots_cmd();
   void slots_timeout();
   void slots_progressBar(int);
private slots:
    void on_refresh_clicked();
    void on_comboBox_deviceList_activated(int index);

private:
    ///////////////////////////tab about//////////////////////
//    QAction* action_about_update;
    void initializeTabAbout();
    void slots_about_update();

    ///////////////////////////tab copy///////////////////
    //    QAction* action_copy_default;
    QStringList stringlist_output_size;
    ScalingSettingKeyboard* keyboard_scaling;
    CopiesSettingKeyboard* keyboard_copies;
    void initializeTabCopy();
    void updateCopy();

    //////////////////////////tab setting///////////////////
    int wifi_encryptionType;
    QString wifi_ssid;
    QString wifi_password;
    int wifi_wepIndex;
    QString wifi_ms_password;
    QString wifi_sw_password;
    int wifi_ms_wepIndex;
    int wifi_sw_wepIndex;
#define NUM_OF_APLIST 10
    int wifi_sw_encryptionType[NUM_OF_APLIST];
    QString machine_wifi_ssid;
//    QString machine_wifi_password;

    bool passwd_checked;
    QString passwd;
    void initializeTabSetting();
    bool wifi_validate_ssidPassword();
    void wifi_update_encryptionType();
    void wifi_update_Data();
    void wifi_update();
    void slots_wifi_applyDo();
    void slots_wifi_applyDone();
    void slots_passwd_setDo();
    void slots_passwd_setDone();
    void slots_wifi_get();
    void slots_wifi_getAplist();

private slots:
    //////////////////tab about///////////////////////////
    /////////////////tab copy///////////////////////////////
    void slots_copy_combo(int);
    void slots_copy_pushbutton();
    void slots_copy_radio(bool);
    void slots_copy_keyboard(QString);
    //////////////////////////tab setting///////////////////
    void slots_wifi_radiobutton(bool);
    void slots_wifi_textChanged(const QString &arg1);
    void slots_wifi_checkbox(bool);
};

#endif // MAINWIDGET_H
