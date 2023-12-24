#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringListModel>
#include <QListView>
#include <QMap>
#include<QList>
#include"dialog.h"
#include"motorconf.h"

#include"serialportmanager.h"
#include<memory>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
struct MotorInfo
{
    double ratio_;
    QString unit_;
    QString nickname_;
    double pos_;
    double vel_;
    bool connected_;
    MotorInfo():ratio_(2),unit_("deg"),nickname_("none"),pos_(0),vel_(0),connected_(false){}
    MotorInfo(double ratio, QString unit, QString nickname, double pos, double vel,bool connected)
        : ratio_(ratio), unit_(unit), nickname_(nickname), pos_(pos),vel_(vel),connected_(connected){}
};

struct Command
{
    int id;
    QByteArray cmd;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QMap<int,MotorInfo> motorInfo;
    SerialPortManager manager;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void handleRead(QByteArray dataBA);
private slots:
    void on_pushButtonConnections_clicked();
    void updateLabel();
    void on_pushButtonRun_clicked();

    void on_pushButtonUpdate_clicked();

    void on_pushButtonStop_clicked();

    void on_pushButtonAutoConnect_clicked();

    void on_pushButtonAutoConfigure_clicked();

    void on_comboBoxId_currentTextChanged(const QString &arg1);


    void on_pushButtonNew_clicked();

    void on_pushButtonAppend_clicked();


    void on_pushButtonRemove_clicked();

    void on_pushButtonExecute_clicked();

    void on_pushButtonLoad_clicked();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<Dialog> dlg;
    std::unique_ptr<MotorConf> conf;
    QStringListModel *model;


    QMap<int,double> motor_pos;
    QList<QList<Command>> commands;
    QList<Command> current_command;

    void UpdateListView();
};

QByteArray toByteArray(QString Command);

QByteArray generateCommand(int motorId, QString mode, double angle, double speed, bool query,bool stop=false);

#endif // MAINWINDOW_H
