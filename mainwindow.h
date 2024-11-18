#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcustomplot.h"
#include "ComPort.h"

#include <QDebug>
#include <QMainWindow>
#include <QFile>
#include <QTableWidgetItem>
#include <QTime>
#include <QMap>
#include <QVector>
#include <cstring>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define LOG_FILE_NAME_CMD   "cmd.log"
#define LOG_FILE_NAME_POINT "points.log"

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

struct PointDesc {
    int _offset_x;
    int _size_x;
    int _offset_y;
    int _size_y;
    unsigned char _cmd;    //without 0x80 and 0x40
    QString fileName;   //ooooh
    PointDesc(int offset_x, int size_x, int offset_y, int size_y) :  _offset_x(offset_x),
                                                                    _size_x(size_x),
                                                                    _offset_y(offset_y),
                                                                    _size_y(size_y) {}
};

struct PointsPos {
    QVector<PointDesc> _points;    /*offset for x and y points in cmd*/
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
    void test_ProcessCmd();
    void test_ProcessData();

    PointsPos pointForGraph;
    void processCmd(unsigned char* cmd);    //from sync byte

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots :
    void OpenPort();
    void ShowObtainedData(const QByteArray& data);
    void WriteDataTofile(const unsigned char* data);
    void ProcessData(const QByteArray& data);   //here need decide what to do with new data... need put to the thread...

signals :
    void sig_NewCmd(const unsigned char* cmd);

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
