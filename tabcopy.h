#ifndef TABCOPY_H
#define TABCOPY_H

#include <QWidget>
#include "app/vop_protocol.h"

namespace Ui {
class TabCopy;
}

class MainWidget;
class DeviceManager;
class CopiesSettingKeyboard;
class ScalingSettingKeyboard;
class TabCopy : public QWidget
{
    Q_OBJECT

public:
    explicit TabCopy(MainWidget* widget,DeviceManager* dm ,QWidget *parent = 0);
    ~TabCopy();
protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::TabCopy *ui;
    MainWidget* main_widget;
    DeviceManager* device_manager;
    ScalingSettingKeyboard* keyboard_scaling;
    CopiesSettingKeyboard* keyboard_copies;

    QStringList stringlist_output_size;
    copycmdset* pCopyPara;
    bool device_status;
    bool idCard_mode;
    copycmdset copyPara;

    void updateCopy();
    void cmdResult_getDeviceStatus(int err);

public slots:
   void slots_cmd_result(int ,int);

private slots:
    void slots_copy_combo(int);
    void slots_copy_pushbutton();
    void slots_copy_radio(bool);
    void slots_copy_keyboard(QString);

    void on_copy_clicked();
    void on_IDCardCopy_clicked();
    void on_btn_default_clicked();
};

#endif // TABCOPY_H
