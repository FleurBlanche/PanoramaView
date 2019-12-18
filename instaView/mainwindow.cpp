#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "showthread.h"
#include <QImage>
#include <QLineEdit>
#include <QStatusBar>
#include <QMenuBar>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <iostream>
#include <QTimer>
#include <QtOpenGL/QGL>
#include "myopenglwidget.h"

QString socketHostIpAddr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //set pointers
    openBtn = ui->centralwidget->findChild<QPushButton *>("openBtn");
    ipAddr = ui->centralwidget->findChild<QLineEdit *>("ipAddr");
    imageLabel = ui->centralwidget->findChild<QLabel *>("imageLabel");
    fpsText = ui->centralwidget->findChild<QTextEdit *>("fpsText");
    //openGLWidget = ui->centralwidget->findChild<QOpenGLWidget *>("openGLWidget");
    menuBar = ui->menubar;
    statusBar = ui->statusBar;

    //set style
    QString btnstyle = "QPushButton{background-color:#F0F8FF;color: black;border:black;border-radius:8px;}"
                    "QPushButton:hover{background-color:#87CEEB; color: white;}"
                    "QPushButton:pressed{background-color:#7FFF00;}"
                    "QToolTip{background-color:white; color:black;}";
    openBtn->setStyleSheet(btnstyle);

    //fps
    QTimer* fTimer = new QTimer(this);
    fTimer->setInterval(timeOut);
    fTimer->start();
    connect(fTimer,SIGNAL(timeout()),this,SLOT(timer_timeout()));

    //
    openGLWidget = new myOpenGLWidget();
    openGLWidget->setGeometry(50,50,800,600);
    openGLWidget->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openBtn_clicked()
{
    //open stream button clicked
    socketHostIpAddr = ipAddr->text();
    std::cout << "Input ip address: " << socketHostIpAddr.toLocal8Bit().toStdString() << std::endl;

    //init ffmpeg thread
    if(videoStream != nullptr){
        //close the current stream and init it again
        videoStream->stopBroad();
        disconnect(videoStream,SIGNAL(sendImage(QImage)) ,this,SLOT(showStream(QImage)));
        while(videoStream->isExit == false);
        videoStream = nullptr;
    }
    videoStream = new showThread;
    connect(videoStream,SIGNAL(sendImage(QImage)) ,this,SLOT(showStream(QImage)),Qt::UniqueConnection);
    videoStream->start();
}

void MainWindow::showStream(QImage image){
    frameCount += 1;
    //pix=QPixmap::fromImage(image.scaled(image.width(),image.height()));
    //pix = pix.scaled(imageLabel->size().width(), imageLabel->size().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(QPixmap::fromImage(image.scaled(imageLabel->size().width(), imageLabel->size().height())));
}

void MainWindow::timer_timeout(){
    //fps
    int fps = frameCount;
    frameCount = 0;
    fpsText->setText(QString::number(fps));
}
