#include "serialportmanager.h"
#include <QDebug>
#include"mainwindow.h"
SerialPortManager::SerialPortManager(MainWindow *parent)
{
    mainWindow = parent;
}

bool SerialPortManager::openPort(int portId, const QString& portName, qint32 baudRate)
{
    // port already exist
    for(auto it=serialPortMap.begin();it!=serialPortMap.end();++it)
    {
        if(it.value()->portName()==portName)
        {
            qWarning() << "this port is already opened.";
            it.value()->close();
            serialPortMap.remove(it.key());
            break;
        }
    }

    // motor id already exists
    if (serialPortMap.contains(portId)) {
        qWarning() << "motor ID" << portId << " already existed.";
        serialPortMap[portId]->close();
        serialPortMap.remove(portId);
    }

    QSerialPort *serialPort = new QSerialPort(this);
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);

    if (serialPort->open(QIODevice::ReadWrite)) {
        serialPortMap.insert(portId, serialPort);

        // Connect signals for data handling
        connect(serialPort, &QSerialPort::readyRead, this, [this, portId]() {
            handleReadyRead(portId);
        });

        return true;
    } else {
        qDebug("cannot open port");
        delete serialPort;
        return false;
    }
}

void SerialPortManager::closePort(int portId) {
    if (serialPortMap.contains(portId)) {
        QSerialPort* serialPort = serialPortMap.take(portId);
        disconnect(serialPort, nullptr, this, nullptr);
        serialPort->close();
        delete serialPort;
    }
}

void SerialPortManager::sendData(int portId, const QByteArray& data) {
    if (serialPortMap.contains(portId)) {
        QSerialPort* serialPort = serialPortMap.value(portId);
        serialPort->write(data);
    } else {
        qWarning() << "Port with ID" << portId << "is not open.";
    }
}

void SerialPortManager::handleReadyRead(int portId) {
    if (serialPortMap.contains(portId)) {
        QSerialPort* serialPort = serialPortMap.value(portId);

        QByteArray dataBA;
        dataBA = serialPort->readAll();

        mainWindow->handleRead(dataBA);

    }
}

void SerialPortManager::handleError(QSerialPort::SerialPortError error) {
    switch (error) {
        case QSerialPort::NoError:
            // No error occurred, do nothing or log it
            break;

        case QSerialPort::DeviceNotFoundError:
            qWarning() << "Device not found. Please check the connection.";
            // Handle device not found error
            break;

        case QSerialPort::PermissionError:
            qWarning() << "Permission error. Check permissions to access the device.";
            // Handle permission error
            break;

        case QSerialPort::ResourceError:
            qWarning() << "Resource error. Serial port disconnected.";
            // Handle serial port disconnect event
            // You may want to attempt to reopen the port or take other actions
            break;

        case QSerialPort::UnsupportedOperationError:
            qWarning() << "Unsupported operation error.";
            // Handle unsupported operation error
            break;

        case QSerialPort::UnknownError:
        default:
            qWarning() << "An unknown error occurred:" << error;
            // Handle other or unknown errors
            break;
    }
}

bool SerialPortManager::testPort(const QString& portName, qint32 baudRate, const QByteArray& testMessage)
{
    QSerialPort *serialPort = new QSerialPort(this);
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    if(serialPort->isOpen())
    {
        delete serialPort;
        return false;
    }
    if (!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open port" << portName;
        return false;
    }
    // Send test message
    serialPort->write(testMessage);
    serialPort->waitForBytesWritten(100); // Adjust the timeout as needed

    // Wait for and read the response
    if (serialPort->waitForReadyRead(100))
    {
        // Adjust the timeout as needed
        QByteArray responseData = serialPort->readAll();
        QString response = QString::fromUtf8(responseData);

        qDebug() << "Response from" << portName << ":" << response;

        // Check if the response matches the expected response
        if (response.size()>0)
        {
            serialPort->close();
            delete serialPort;
            return true;
        }
    }

    serialPort->close();
    delete serialPort;
    return false;
}
