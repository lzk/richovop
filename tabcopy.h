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
struct CopyData;
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

public:
    void set_copy_data(struct CopyData* cd);

private:
    MainWidget* main_widget;
    DeviceManager* device_manager;
    ScalingSettingKeyboard* keyboard_scaling;
    CopiesSettingKeyboard* keyboard_copies;

    QStringList stringlist_output_size;
    copycmdset* pCopyPara;
    struct CopyData* copy_data;

private:
    copycmdset copyPara;

    void updateCopy();
    void cmdResult_getDeviceStatus(int err);
    bool cmd_err_handler(int err);

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
