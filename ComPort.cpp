#include "ComPort.h"
ComPort::ComPort()
{
}
ComPort::~ComPort()
{
    qDebug("End power of the Thread!");
    emit finished_Port();//Сигнал о завершении работы
}
void ComPort::ClosePort() {
    if(thisPort.isOpen()) {
        thisPort.close();
        qDebug("port is closed");
//        error_(SettingsPort.name.toLocal8Bit() + " >> Закрыт!\r");
    }
}
void ComPort::OpenPort(void) {
    thisPort.setPortName(SettingsPort.name);
    if (thisPort.open(QIODevice::ReadWrite)) {
       if (thisPort.setBaudRate(SettingsPort.baudRate)
               && thisPort.setDataBits(SettingsPort.dataBits)//DataBits
               && thisPort.setParity(SettingsPort.parity)
               && thisPort.setStopBits(SettingsPort.stopBits)
               && thisPort.setFlowControl(SettingsPort.flowControl))
        {
            if (thisPort.isOpen()){
               printf("com port is opened!\n");
            }
        }
        else {
            printf("com port is closed\n");
            thisPort.close();
       }
    } else {
       thisPort.close();
       error_(thisPort.errorString().toLocal8Bit());
    }
}
void ComPort::Write_Settings_Port(QString name, int baudrate, int DataBits, int Parity, int StopBits, int FlowControl)
{
    SettingsPort.name = name;
    SettingsPort.baudRate = (QSerialPort::BaudRate) baudrate;
    SettingsPort.dataBits = (QSerialPort::DataBits) DataBits;
    SettingsPort.parity = (QSerialPort::Parity) Parity;
    SettingsPort.stopBits = (QSerialPort::StopBits) StopBits;
    SettingsPort.flowControl = (QSerialPort::FlowControl) FlowControl;
}
void ComPort::process_Port()
{
    qDebug("Nice to meet you in Thread!");
    connect(&thisPort, SIGNAL(readyRead()), this, SLOT(ReadInPort()));    //подключаем   чтение с порта по сигналу readyRead()
}
void ComPort::WriteToPort(QByteArray data)
{
    if(thisPort.isOpen()){
        thisPort.write(data);
    }
}
void ComPort::handleError(QSerialPort::SerialPortError error)
{
    if ( (thisPort.isOpen()) && (error == QSerialPort::ResourceError)) {
        error_(thisPort.errorString().toLocal8Bit());
        ClosePort();
    }
}
void ComPort::ReadInPort()
{
//    printf("Data come\n");
    static QByteArray data;
    data.append(thisPort.readAll());
    if (data.size() < 1)
        return;
    if (data.at(data.size() - 1) == '\n') {
        QByteArray data_out(data.data(), data.size()-2); //I think when data will sent, I can remove them
        emit DataIsReady(data_out);
        data.clear();
//        qDebug(data_out);
    }
}
