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

void MainWindow::processCmd(unsigned char *cmd)
{
    if (cmd[pos_CMD] == 0x88) {
//        pointForGraph._points.push_back(PointDesc());
        //1byte - ca
        //2byte - len
        //3byte - cmd
        //4byte - offset x0
        //5byte - size point x0
        //6byte - offset y0
        //7byte - size point y0

        pointForGraph._points.push_back(PointDesc(cmd[3], cmd[4], cmd[5], cmd[6]));

        return;
    }
    /*write to file*/
    if (cmd[pos_CMD] & 0x40) {
        auto iter = pointForGraph._points.begin();
        for (; iter != pointForGraph._points.end(); iter++) {

        }
    }
    /*build point*/
    if (cmd[pos_CMD] & 0x80 && cmd[pos_CMD] != 0x88) {
        static double max_temperature = 0;
        auto iter = pointForGraph._points.begin();
        for (; iter != pointForGraph._points.end(); iter++) {
            if (ui->W_Plot->graphCount() == 0) {
                ui->W_Plot->addGraph();
                ui->W_Plot->xAxis->setRange(0, 100);
                ui->W_Plot->yAxis->setRange(-10, 80);
            }
            unsigned int time = cmd[iter->_offset_x + 3] | (cmd[iter->_offset_x + 1 + 3] << 8) | (cmd[iter->_offset_x + 2 + 3] << 16) | (cmd[iter->_offset_x + 3 + 3] << 24);
            int  temperature = cmd[iter->_offset_y + 3] | (cmd[iter->_offset_y + 1 + 3] << 8) | (cmd[iter->_offset_y + 2 + 3] << 16) | (cmd[iter->_offset_y + 3 + 3] << 24);

            max_temperature = max_temperature < (double)temperature / 100.0 ? (double)temperature/100.0 : max_temperature;

            ui->W_Plot->graph(0)->addData((double)time / 1000.0, (double)temperature/100.0);
            ui->W_Plot->replot();
            //we find cmd with format... and start write
        }

    }
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
                    curSize = 0;
                    curStatus = Status::waitLen;
                }
            break;
            case Status::waitLen :
                cmd[pos_LEN] = data[i];
                curStatus = Status::waitCmd;
                curSize = data[i] - 2   /*cmd, len...*/;
            break;
            case Status::waitCmd :
                cmd[pos_CMD] = data[i];
                curStatus = Status::procData;
            break;
            case Status::procData :
                if ((data.size() - i) >= curSize) {
                    std::memcpy(&cmd[pos_DATA + (cmd[pos_LEN] - curSize)], &data.data()[i], cmd[pos_LEN]);
                    i += cmd[pos_LEN];
                    if (Checkxor(cmd, cmd[pos_LEN]) == 0) { /*don't check xor*/}

                    qDebug() << "cmd is valid :" << QByteArray((char*)&cmd[0], (unsigned int)cmd[pos_LEN]);
                    processCmd(cmd);
                    curStatus = Status::waitSync;
                }
                else {
                    std::memcpy(&cmd[pos_DATA + (cmd[pos_LEN] - curSize)], &data.data()[i], (data.size() - i));
                    curSize -= (data.size() - i);   //<-- need check
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

//    test_ProcessData();
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
                           QSerialPort::BaudRate::Baud9600,
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

//    ui->tableWidget->scrollToBottom();

}

void MainWindow::WriteDataTofile(const unsigned char *data)
{

}

void MainWindow::test_ProcessCmd()
{
    unsigned char cmd[] = {0xCA, 0x07, 0x88, 0, 4, 4, 4, 0xFF};
    processCmd(cmd);    //size of cmd is 2nd byte

    unsigned char cmd1[] = {0xCA, 0x0D, 0x81, 0x01,0x00,0x00,0x00, 0xAA,0x00,0x00,0x00, 0xFF,0xFF, 0xFF};
    processCmd(cmd1);
}

void MainWindow::test_ProcessData()
{
    ProcessData(this->convertStrToHex("CA, 07, 88, 00, 04, 04, 04, FF"));
    ProcessData(this->convertStrToHex("CA, 0D, 81, 01,00,00,00, AA,00,00,00, FF,FF, FF"));

    ProcessData(this->convertStrToHex("CA, 0D, 81, 01,00,0000, AA00,00,00"));
    ProcessData(this->convertStrToHex("ff ff ff"));
}

