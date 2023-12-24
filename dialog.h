#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringListModel>
#include <QListView>

namespace Ui {
class Dialog;
}
class MainWindow;
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void update(){on_pushButtonUpdate_clicked();}

public slots:
    void on_pushButtonAdd_clicked();

    void on_pushButtonRefreshPort_clicked();

    void on_pushButtonDelete_clicked();

    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_pushButtonUpdate_clicked();

    void on_pushButtonClearAll_clicked();

    void on_pushButtonAutoconnect_clicked();

private:
    Ui::Dialog *ui;
    MainWindow *mainWindow;

    QSerialPortInfo info;
    QStringListModel *model;
};

#endif // DIALOG_H
