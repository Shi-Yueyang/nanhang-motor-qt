#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <cstdlib> // For rand function
#include <ctime>   // For srand function
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , manager(this)
{
    this->setFixedSize(787,427);

    // child window
    dlg =std::make_unique<Dialog>(this);
    conf = std::make_unique<MotorConf>(this);
    // motor information setup

    // set up ui
    ui->setupUi(this);
    ui->comboBoxMode->addItem("continues");
    ui->comboBoxMode->addItem("impulse");
    ui->comboBoxId->addItem(" ");
    ui->comboBoxController->addItem("Velocity feedforward");

    // update motor position
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateLabel);
    timer->start(10);

    // list view
    model = new QStringListModel(this);
    ui->listViewCmds->setModel(model);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleRead(QByteArray dataBA)
{
    if(dataBA.size()<8)
    {
        return;
    }
    QVector<uint8_t> data;
    for (int i = 0; i < dataBA.size(); ++i)
    {
        int character = static_cast<uint8_t>(dataBA.at(i));
        data.append(character);
    }
    QString message;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) {
            message += " ";  // Add space between elements
        }
        message += QString::number(data.at(i),16).rightJustified(2, '0');;
    }

    QStringList hexNumbers = message.split(" ");
    int id = hexNumbers[0].toInt();
    double angleDouble = 0;

    QString turn1 = hexNumbers[2];
    QString turn2 = hexNumbers[3];
    QString turn = turn1+turn2;
    double turnInt = turn.toInt(nullptr,16)-0x8000;

    QString angle1 = hexNumbers[4];
    QString angle2 = hexNumbers[5];
    QString angle = angle1+angle2;
    angleDouble = angle.toDouble()/4000.0*360.0 + 360.0*turnInt;
    motor_pos.insert(id, angleDouble);
    motorInfo[id].connected_=true;
    motorInfo[id].pos_=angleDouble;
}

void MainWindow::updateLabel()
{
    for(auto it=manager.serialPortMap.begin();it!=manager.serialPortMap.end();++it)
    {
        QString tmp=ui->comboBoxMode->currentText();
        QByteArray cmd = generateCommand(it.key(),tmp,0,0,true);
        manager.sendData(it.key(),cmd);
    }
    ui->textBrowserPos->clear();
    QThread::msleep(10);
    for(auto it=motorInfo.begin();it!=motorInfo.end();++it)
    {
        if(it.value().connected_)
            ui->textBrowserPos->append(QString("%1: %2%3").arg(it.value().nickname_).arg(it.value().pos_/it.value().ratio_).arg(it.value().unit_));

    }
}


void MainWindow::on_pushButtonRun_clicked()
{
    bool ok;
    int motorId = (ui->comboBoxId->currentText()).toInt(&ok);
    if(!ok) return;
    QString modeString = ui->comboBoxMode->currentText();
    QString target = ui->lineEditTarget->text();
    QString speedInput = ui->lineEditSpeed->text();
    double angle = target.toDouble(&ok)*motorInfo[motorId].ratio_;
    if(!ok) return;
    double speed = speedInput.toDouble(&ok);
    if(!ok) return;

    // generate command
    QByteArray cmd = generateCommand(motorId,modeString,angle,speed,false);
    manager.sendData(motorId,cmd);

}

void MainWindow::on_pushButtonUpdate_clicked()
{
    ui->comboBoxId->clear();
    for(auto it=manager.serialPortMap.begin();it!=manager.serialPortMap.end();++it)
    {
        int motorid = it.key();
        ui->comboBoxId->addItem(QString::number(motorid));
    }
}

void MainWindow::on_pushButtonStop_clicked()
{
    for(auto it=manager.serialPortMap.begin();it!=manager.serialPortMap.end();++it)
    {
        int motorId =it.key();
        QByteArray cmd = generateCommand(motorId," ",0,0,false,true);
        manager.sendData(motorId,cmd);
    }

}

QByteArray toByteArray(QString Command)
{
    QByteArray byteArray;
    QStringList hexList = Command.split(" ", Qt::SkipEmptyParts);

    for (const QString& hex : hexList)
    {
        bool ok;
        byteArray.append(static_cast<char>(hex.toInt(&ok, 16)));

        if (!ok)
        {
            // Handle conversion error (invalid hex number)
            qDebug() << "Error: Invalid hex number -" << hex;
            break;
        }
    }
    return byteArray;
}

QByteArray generateCommand(int motorId, QString mode, double angle, double speed, bool query, bool stop)
{
    QString idstr = QString::number(motorId);
    if(stop)
    {
        QString command = idstr+" 53 00 00 00 00 00 0A";
        return toByteArray(command);
    }

    if(query)
    {
        QString command = idstr+" 51 00 00 00 00 00 0A";
        return toByteArray(command);
    }
    // mode
    QString modestr;
    if (mode == "continues")
    {
        modestr = "02";
    }
    else
    {
        modestr = "03";
    }
    // target
    int turns = static_cast<int>(angle / 360)+0x8000;
    if(angle>=360||angle<=-360)
        angle /=360;
    if(angle<0)
    {
        angle = 360+angle;
        turns-=1;
    }
    double remainingAngle = fmod(angle, 360.0);
    QString turnsHexString = QString("%1").arg(turns, 4, 16, QChar('0'));
    turnsHexString = turnsHexString.right(4);
    int tmp = static_cast<int>((remainingAngle / 360.0) * 4000);
    QString remainingAngleString = QString("%1").arg(tmp, 4, 10, QChar('0'));
    QString targetString = turnsHexString.mid(0,2)+" "+turnsHexString.mid(2,2)
                           +" "+remainingAngleString.mid(0,2)+" "+remainingAngleString.mid(2,2);

    int hexSpeed = static_cast<int>((speed / 100.0) * 255);
    QString speedString = QString("%1").arg(hexSpeed, 2, 16, QChar('0')).toUpper();
    QString Command=idstr+" "+modestr+" "+targetString+" "+speedString+" 0A";
    qDebug()<<"cmd: "<<Command;

    return toByteArray(Command);
}


void MainWindow::on_pushButtonAutoConnect_clicked()
{
    QString infotxt = ui->labelInfo->text();
    ui->labelInfo->setText("connecting");
    conf->on_pushButtonLoadConfig_clicked();
    dlg->on_pushButtonAutoconnect_clicked();
    on_pushButtonUpdate_clicked();
    ui->labelInfo->setText(infotxt);

}

void MainWindow::on_pushButtonConnections_clicked()
{

    dlg->show();
    dlg->update();
}

void MainWindow::on_pushButtonAutoConfigure_clicked()
{
    conf->show();
    conf->update();
}


void MainWindow::on_comboBoxId_currentTextChanged(const QString &arg1)
{
    int id = arg1.toInt();
    if(motorInfo.value(id).connected_)
        ui->labelInfo->setText(motorInfo.value(id).nickname_+"   "+motorInfo.value(id).unit_);
}

void MainWindow::on_pushButtonNew_clicked()
{
    commands.push_back({});
}

void MainWindow::on_pushButtonAppend_clicked()
{
    bool ok;
    int motorId = (ui->comboBoxId->currentText()).toInt(&ok);
    if(!ok) return;
    QString modeString = ui->comboBoxMode->currentText();
    QString target = ui->lineEditTarget->text();
    QString speedInput = ui->lineEditSpeed->text();
    double angle = target.toDouble(&ok)*motorInfo[motorId].ratio_;
    if(!ok) return;
    double speed = speedInput.toDouble(&ok);
    if(!ok) return;


    // generate command
    QByteArray cmd = generateCommand(motorId,modeString,angle,speed,false);
    if(commands.empty())
        commands.push_back({});
    commands.back().push_back({motorId,cmd});
    UpdateListView();
}

void MainWindow::on_pushButtonRemove_clicked()
{
    QModelIndex selectedIndex = ui->listViewCmds->currentIndex();
    if (selectedIndex.isValid()) {
        int selectedRow = selectedIndex.row();
        qDebug() << "Selected Item Index: " << selectedRow;
        model->removeRow(selectedRow);
        commands.removeAt(selectedRow);
    }
    UpdateListView();
}

void MainWindow::UpdateListView()
{
    QStringList itemList;
    model->setStringList(itemList);
    for(int i=0;i<commands.size();i++)
    {
        QString text;
        for(int j=0;j<commands[i].size();j++)
        {
            text += commands[i][j].cmd.toHex();
            if(j!=commands[i].size()-1)
                text += "\n";
        }
        itemList<<text;
    }
    model->setStringList(itemList);
}

void MainWindow::on_pushButtonExecute_clicked()
{
    QModelIndex selectedIndex = ui->listViewCmds->currentIndex();

    if (selectedIndex.isValid())
    {
        int selectedRow = selectedIndex.row();

        for(int i=0;i<commands[selectedRow].size();i++)
        {
            manager.sendData(commands[selectedRow][i].id,commands[selectedRow][i].cmd);
        }
    }
}

void MainWindow::on_pushButtonLoad_clicked()
{
    QString filePath = "C:\\Users\\DELL\\source\\qt\\multi_mortors\\commands.json";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open the file";
        return ;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    commands.clear();
    // Check if parsing was successful and the root is an array
    if (jsonDoc.isArray())
    {
        QJsonArray jsonArray = jsonDoc.array();

        // Iterate through each object in the array
        for (const auto& jsonObject : jsonArray)
        {

            QString name = jsonObject.toObject()["name"].toString();
            QJsonArray cmdArray = jsonObject.toObject()["commands"].toArray();
            qDebug()<<name;
            for(const auto& cmdjson:cmdArray)
            {
                // Access values from each object
                int motorId = cmdjson.toObject()["id"].toInt();
                QString modeString = cmdjson.toObject()["mode"].toString();
                double angle = cmdjson.toObject()["angle"].toDouble();
                double speed = cmdjson.toObject()["speed"].toDouble();
                qDebug()<<speed;
                QByteArray cmd = generateCommand(motorId,modeString,angle,speed,false);
                if(commands.empty())
                    commands.push_back({});
                commands.back().push_back({motorId,cmd});
            }
            on_pushButtonNew_clicked();
        }
    }
    UpdateListView();


}

