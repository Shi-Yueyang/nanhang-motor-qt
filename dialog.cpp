#include "dialog.h"
#include "ui_dialog.h"
#include"mainwindow.h"
#include <QRegularExpression>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    this->setFixedSize(578,329);
    ui->setupUi(this);
    model = new QStringListModel(this);

    mainWindow = qobject_cast<MainWindow*>(parentWidget());
    ui->listViewComs->setModel(model);
    ui->lineEditId->setText("41");


    connect(ui->listViewComs->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &Dialog::onCurrentChanged);

    on_pushButtonRefreshPort_clicked();

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButtonAdd_clicked()
{
    int id = ui->lineEditId->text().toInt();
    QString port = ui->comboBoxPort->currentText();
    qint32 bdrate = ui->comboBoxBaudRate->currentText().toInt();
    mainWindow->manager.openPort(id,port,bdrate);

    on_pushButtonUpdate_clicked();
}


void Dialog::on_pushButtonRefreshPort_clicked()
{
    //ports
    QList<QSerialPortInfo> ports = info.availablePorts();
    QList<QString> stringPorts;
    for(int i = 0 ; i < ports.size() ; i++){
        stringPorts.append(ports.at(i).portName());
    }
    ui->comboBoxPort->clear();
    ui->comboBoxPort->addItems(stringPorts);

    // Baud Rate Ratios
    QList<qint32> baudRates = info.standardBaudRates(); // What baudrates does my computer support ?
    QList<QString> stringBaudRates;
    for(int i = 0 ; i < baudRates.size() ; i++){
        if (baudRates.at(i) == 115200) {
            stringBaudRates.prepend(QString::number(baudRates.at(i))); // Add to the beginning of the list
        } else {
            stringBaudRates.append(QString::number(baudRates.at(i))); // Add to the end of the list
        }
    }
    ui->comboBoxBaudRate->addItems(stringBaudRates);

    // Data Bits
    ui->comboBoxDataBIts->addItem("8");
    ui->comboBoxDataBIts->addItem("7");

    // Stop Bits
    ui->comboBoxStopBits->addItem("1 Bit");
    ui->comboBoxStopBits->addItem("1,5 Bits");
    ui->comboBoxStopBits->addItem("2 Bits");

    // Parities
    ui->comboBoxParity->addItem("No Parity");
    ui->comboBoxParity->addItem("Even Parity");
    ui->comboBoxParity->addItem("Odd Parity");
    ui->comboBoxParity->addItem("Mark Parity");
    ui->comboBoxParity->addItem("Space Parity");



}


void Dialog::onCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    // Check if the selection is not empty and update your QMap

    if (current.isValid()) {
        QString selectedItem = model->data(current, Qt::DisplayRole).toString();
        qDebug() << "Selected Item:"<<selectedItem;
        QRegularExpression regex("Port ID: (\\d+),");
        QRegularExpressionMatch match = regex.match(selectedItem);
        QString portId = match.captured(1); // Capturing group 1 contains the Port ID
        qDebug() << "Port ID:" << portId.toInt();


    }
}


void Dialog::on_pushButtonDelete_clicked()
{
    QModelIndexList selectedIndexes = ui->listViewComs->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        QString selectedItem = model->data(selectedIndexes.first(), Qt::DisplayRole).toString();
        qDebug() << "Selected Item:" << selectedItem;
        QRegularExpression regex("Port ID: (\\d+),");
        QRegularExpressionMatch match = regex.match(selectedItem);
        int portId = match.captured(1).toInt(); // Capturing group 1 contains the Port ID

        QSerialPort* port = mainWindow->manager.serialPortMap[portId];
        port->close();
        mainWindow->manager.serialPortMap.remove(portId);
        delete port;
        on_pushButtonUpdate_clicked();
    } else {
        qDebug() << "No item selected.";
    }
}


void Dialog::on_pushButtonUpdate_clicked()
{
    QStringList itemList;
    model->setStringList(itemList);

    for (auto it = mainWindow->manager.serialPortMap.begin(); it != mainWindow->manager.serialPortMap.end(); ++it) {
        int portId = it.key();
        QSerialPort* serialPort = it.value();

        QString portInfo = QString("Motor ID: %1, Port Name: %2").arg(portId).arg(serialPort->portName());
        itemList<<portInfo;
        // Print information about the QSerialPort
        qDebug() << portInfo;

    }
    model->setStringList(itemList);

}


void Dialog::on_pushButtonClearAll_clicked()
{
    for (auto it = mainWindow->manager.serialPortMap.begin(); it != mainWindow->manager.serialPortMap.end(); ++it) {
        it.value()->close();
    }
    mainWindow->manager.serialPortMap.clear();
    on_pushButtonUpdate_clicked();
}


void Dialog::on_pushButtonAutoconnect_clicked()
{
    QList<int> motor{41,42,43,44,45,46};
    qint32 bdrate = ui->comboBoxBaudRate->currentText().toInt();

    on_pushButtonClearAll_clicked();
    QList<QSerialPortInfo> ports = info.availablePorts();
    QList<QString> stringPorts;
    for(int i = 0 ; i < ports.size() ; i++){
        stringPorts.append(ports.at(i).portName());
    }
    for(int m:motor)
    {
        for(int i=0;i<stringPorts.size();i++)
        {
            QString portName = stringPorts[i];
            QByteArray cmd = generateCommand(m," ",0,0,true);
            if(mainWindow->manager.testPort(stringPorts[i],bdrate,cmd))
            {
                mainWindow->manager.openPort(m,stringPorts[i],bdrate);
                stringPorts.removeAt(i);
                break;
            }
        }
    }
    on_pushButtonUpdate_clicked();
}

