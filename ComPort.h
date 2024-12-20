#ifndef COMPORT_H
#define COMPORT_H

#include <QDebug>
#include    <QObject>
#include    <QSerialPort>
#include    <QString>
#include <stdio.h>
#include <string.h>
struct Settings {//Структура с настройками порта
    QString name;
    qint32 baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
};
class ComPort : public QObject{
    Q_OBJECT
private:
    int b;
    int statusPort;
public:
//    explicit COM_PORT(QObject *parent = 0);
    ComPort();
    ~ComPort();
    QSerialPort thisPort;   //так делать нельзя....
    Settings SettingsPort;  //так тоже делать нельзя....
    bool isOpened() const {
        return statusPort;
    }
signals:
    void finished_Port(); //Сигнал закрытия класса
    void error_(QString err);//Сигнал ошибок порта
    void outPort(QString data); //Сигнал вывода полученных данных
    void DataIsReady(const QByteArray& data);         //сигнал готовности данных
public slots:
    void ClosePort(); // Слот отключения порта
    void OpenPort(void); // Слот подключения порта
    void setParamPort(QString name, int baudrate, int DataBits, int Parity, int StopBits, int FlowControl);// Слот занесение настроек порта в класс
    void process_Port();                //Тело
    void WriteToPort(const QByteArray& data); // Слот отправки данных в порт
private slots:
    void ReadInPort(); //Слот чтения из порта по ReadyRead
    void handleError(QSerialPort::SerialPortError error);//Слот обработки ощибок
};
#endif // COMPORT_H
