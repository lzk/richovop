/////////////////////////////////////////
/// Author:Jacky Liang
/// Version:
/////////////////////////////////////////
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
namespace Ui {
class MainWindow;
}

class MainWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void refresh();
    static QMessageBox::StandardButton messagebox_exec(const QString &text,
                                                QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                               QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

protected:
    virtual void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
    MainWidget* mainWidget;

private slots:
    void slots_exit();
    void slots_desktopResized(int);
};

#endif // MAINWINDOW_H
