#ifndef SHOWTHREAD_H
#define SHOWTHREAD_H

#include <QThread>
#include <QMainWindow>

class showThread: public QThread
{
    Q_OBJECT
public:
    bool stopBit;
    bool exitDecode = false;
    bool takePic;
    showThread();
    void run(); //声明继承于QThread虚函数 run()
    void stopBroad();
    void setExit(bool);
    void setPic(bool);
    int frameCounts;
    int prevCounts;
    signals:
        void sendImage(QImage image);
};

#endif
