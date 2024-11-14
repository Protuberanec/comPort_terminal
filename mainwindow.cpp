#include "mainwindow.h"
#include "ui_mainwindow.h"


QByteArray MainWindow::convertStrToHex(const QString &text)
{
    QByteArray retData;
    QByteArray temp = text.toLower().toUtf8();
    unsigned char byte = 0;
    for (int i = 0, num_tetrad = 0; i < text.size(); i++) {
        if ((unsigned char)temp[i] <= 0x39 && (unsigned char)temp[i] >= 0x30) {
            byte |= (temp[i] - 0x30) << (4 * ((num_tetrad++ + 1) % 2));
        }
        else if (temp[i] >= 'a' && temp[i] <= 'f'){
            byte |= (0x0A + (temp[i] - 'a')) << (4 * ((num_tetrad++ + 1) % 2));
        }
        if (num_tetrad % 2 == 0 && num_tetrad != 0) {
            retData.append(byte);
            byte = 0;
            num_tetrad = 0;
        }
    }
    return retData;
}
//CA 09 01 81 82 83 84 85 86
void MainWindow::ProcessData(const QByteArray &data)
{
    static Status curStatus = Status::waitSync;
    static unsigned char cmd[64] = {SYNC};
    static unsigned char curSize = 0;

    for (int i = 0; i < data.size(); i++) {
        switch(curStatus) {
            case Status::waitSync :
                if ((unsigned char)data[i] == (unsigned char)SYNC) {
                    curStatus = Status::waitLen;
                }
            break;
            case Status::waitLen :
                cmd[pos_LEN] = data[i];
                curStatus = Status::waitCmd;
                curSize = data[i] - 3   /*sync, cmd, len...*/;
            break;
            case Status::waitCmd :
                cmd[pos_CMD] = data[i];
                curStatus = Status::procData;
            break;
            case Status::procData :
                if ((data.size() - i) >= curSize) {
                    std::memcpy(&cmd[pos_DATA], &data.data()[i], cmd[pos_LEN]);
                    i += cmd[pos_LEN];
                    if (Checkxor(cmd, cmd[pos_LEN]) == 0) { /*don't check xor*/}
                    qDebug() << "cmd is valid :" << QByteArray((char*)&cmd[0], (unsigned int)cmd[pos_LEN]);
                    emit sig_NewCmd(cmd);
                    curStatus = Status::waitSync;
                }
                else {
                    std::memcpy(&cmd[pos_DATA], &data.data()[i], (data.size() - i));
                    curSize -= (data.size() - i);
                    i += (data.size() - i);
                }
            break;
        }
    }


}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(0);
    QStringList headers = {"time", "Data"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    thisPort = new ComPort;
    connect(ui->PB_OpenPort, &QPushButton::clicked, this, &MainWindow::OpenPort);
    connect(ui->PB_Send, &QPushButton::clicked, this, [&](){
        thisPort->WriteToPort(this->convertStrToHex(ui->LE_DataFroPort->text().toUtf8()));
    });
    connect(thisPort, &ComPort::DataIsReady, this, &MainWindow::ShowObtainedData);
    connect(thisPort, &ComPort::DataIsReady, this, &MainWindow::ProcessData);
//    connect(thisPort, &ComPort::DataIsReady, this, &MainWindow::WriteDataTofile);
//    connect(thisPort, &ComPort::DataIsReady, this, &MainWindow::DrawPoint);
}

MainWindow::~MainWindow()
{
    delete thisPort;
    delete ui;
}

void MainWindow::OpenPort()
{
    qDebug() << __func__;
    thisPort->setParamPort(ui->LE_PortName->text(),
                           QSerialPort::BaudRate::Baud115200,
                           QSerialPort::DataBits::Data8,
                           QSerialPort::Parity::NoParity,
                           QSerialPort::StopBits::OneStop,
                           QSerialPort::FlowControl::NoFlowControl);
    thisPort->OpenPort();
    qDebug() << "status port : " << (thisPort->isOpened() == 1 ? "opened" : "closed") ;
}

void MainWindow::ShowObtainedData(const QByteArray &data)
{
//    ui->TB_InData->cou
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
    auto currentTime = QTime::currentTime().toString("HH:mm:ss:zzz");
    QTableWidgetItem *new_item = new QTableWidgetItem(currentTime);
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, new_item);

    new_item = new QTableWidgetItem(QString(data.toHex(' ')));
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, new_item);

    ui->tableWidget->scrollToBottom();

}

void MainWindow::WriteDataTofile(const unsigned char *data)
{

}

void MainWindow::DrawPoint(const unsigned char *data)
{

}


