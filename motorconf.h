#ifndef MOTORCONF_H
#define MOTORCONF_H

#include <QDialog>

namespace Ui {
class MotorConf;
}
class MainWindow;
class MotorConf : public QDialog
{
    Q_OBJECT

public:
    explicit MotorConf(QWidget *parent = nullptr);
    ~MotorConf();

public slots:
    void on_pushButtonAddConfig_clicked();

    void on_pushButtonLoadConfig_clicked();

    void on_pushButtonClearAll_clicked();

private:
    Ui::MotorConf *ui;
    MainWindow *mainWindow;
};

#endif // MOTORCONF_H
