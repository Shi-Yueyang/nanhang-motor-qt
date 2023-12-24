#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H
#include<QMap>
#include <QSerialPort>
#include <QObject>

#include <QSerialPortInfo>
class MainWindow;
class SerialPortManager: public QObject
{
    Q_OBJECT
public:
    QMap<int,QSerialPort*> serialPortMap;

    SerialPortManager(MainWindow *parent);
    bool openPort(int portId, const QString& portName, qint32 baudRate);
    void closePort(int portId);
    void sendData(int portId, const QByteArray& data);
    bool testPort(const QString& portName, qint32 baudRate, const QByteArray& testMessage);
private slots:
    void handleReadyRead(int portId);
    void handleError(QSerialPort::SerialPortError error);
private:
    MainWindow *mainWindow;

};

#endif // SERIALPORTMANAGER_H
