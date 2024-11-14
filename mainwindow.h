#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcustomplot.h"
#include "ComPort.h"

#include <QDebug>
#include <QMainWindow>
#include <QFile>
#include <QTableWidgetItem>
#include <QTime>
#include <cstring>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define SYNC        0xCA
#define pos_SYNC    0x00
#define pos_LEN     0x01
#define pos_CMD     0x02
#define pos_DATA    0x03

enum class Status {
    waitSync = 0,
    waitLen,
    waitCmd,
    procData,
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
private :
    ComPort* thisPort;

    QByteArray convertStrToHex(const QString& text);
    bool Checkxor(const unsigned char* data, int size) {
        unsigned char xors = data[0];
        for (int i = 1; i < size; i++) {
            xors ^= data[i];
        }
        return xors;
    }

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots :
    void OpenPort();
    void ShowObtainedData(const QByteArray& data);
    void WriteDataTofile(const unsigned char* data);
    void DrawPoint(const unsigned char* data);
    void ProcessData(const QByteArray& data);   //here need decide what to do with new data... need put to the thread...

signals :
    void sig_NewCmd(const unsigned char* cmd);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
