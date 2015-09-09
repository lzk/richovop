#ifndef TABCOPY_H
#define TABCOPY_H

#include <QWidget>

class DeviceManager;
class CopiesSettingKeyboard;
class ScalingSettingKeyboard;
namespace Ui {
class TabCopy;
}

class TabCopy : public QWidget
{
    Q_OBJECT

public:
    explicit TabCopy(QWidget *parent = 0);
    ~TabCopy();

private:
    Ui::TabCopy *ui;

    QStringList stringlist_output_size;
    DeviceManager* device_manager;
    ScalingSettingKeyboard* keyboard_scaling;
    CopiesSettingKeyboard* keyboard_copies;

    void updateCopy();
    void copy_button_IDCardCopy();
private slots:
    void slots_copy_combo(int);
    void slots_copy_pushbutton();
    void slots_copy_radio(bool);
    void slots_copy_keyboard(QString);
};

#endif // TABCOPY_H
