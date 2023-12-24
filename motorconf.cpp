#include "motorconf.h"
#include "ui_motorconf.h"
#include"mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QFile>

MotorConf::MotorConf(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MotorConf)
{
    ui->setupUi(this);
    mainWindow = qobject_cast<MainWindow*>(parentWidget());
    ui->comboBoxUnit->addItems({"deg","mm"});
}

MotorConf::~MotorConf()
{
    delete ui;
}

void MotorConf::on_pushButtonAddConfig_clicked()
{
    bool ok;
    int id = ui->lineEditId->text().toInt(&ok);
    if(!ok) return;
    double ratio = ui->lineEditId->text().toDouble(&ok);
    if(!ok) return;
    QString nickname = ui->lineEditNickname->text();
    if(!ok) return;

    QString unit = ui->comboBoxUnit->currentText();
    mainWindow->motorInfo.insert(id,{ratio,unit,nickname,0,0,false});
}


void MotorConf::on_pushButtonLoadConfig_clicked()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString filePath = appDir + "/motorinfo.json";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open the motorinfo file under "<<filePath;
        return ;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    mainWindow->motorInfo.clear();
    if (jsonDoc.isArray())
    {
        QJsonArray jsonArray = jsonDoc.array();

        // Iterate through each object in the array
        for (const auto& jsonObject : jsonArray)
        {
            int id = jsonObject.toObject()["id"].toInt();
            int ratio = jsonObject.toObject()["ratio"].toInt();
            QString unit = jsonObject.toObject()["unit"].toString();
            QString nickname = jsonObject.toObject()["nickname"].toString();
            mainWindow->motorInfo.insert(id,{ratio,unit,nickname,0,0,false});
        }
    }

    // update list view
    QStringList itemList;
    for (const auto& key : mainWindow->motorInfo.keys())
    {
        auto value = mainWindow->motorInfo.value(key);
        ui->textBrowserConf->append(QString("%1: %2").arg(key).arg(value.nickname_));
    }

}


void MotorConf::on_pushButtonClearAll_clicked()
{
    mainWindow->motorInfo.clear();
    ui->textBrowserConf->clear();
}

