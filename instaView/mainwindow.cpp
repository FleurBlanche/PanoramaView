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
#include <QMatrix>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include "myopenglwidget.h"
#include "fishEye.cpp"
#include "helper.cpp"
#include "matAndQImage.cpp"

using namespace std;

QString socketHostIpAddr;

//全局纹理图片
QImage img1;
QImage img2;
QImage img3;
QImage img4;
QImage fullImage;
QImage originImage;
QImage earth;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(50,50,this->width(),this->height());

    FishEyeTest();
    //FishEyeTest1();
    earth = QImage("earth.png");

    //set pointers
    openBtn = ui->centralwidget->findChild<QPushButton *>("openBtn");
    ipAddr = ui->centralwidget->findChild<QLineEdit *>("ipAddr");
    imageLabel = ui->centralwidget->findChild<QLabel *>("imageLabel");
    fpsText = ui->centralwidget->findChild<QTextEdit *>("fpsText");
    //openGLWidget = ui->centralwidget->findChild<QOpenGLWidget *>("openGLWidget");
    menuBar = ui->menubar;
    statusBar = ui->statusBar;
    img1Label = ui->centralwidget->findChild<QLabel *>("img1");
    img2Label = ui->centralwidget->findChild<QLabel *>("img2");
    img3Label = ui->centralwidget->findChild<QLabel *>("img3");
    img4Label = ui->centralwidget->findChild<QLabel *>("img4");
    img5Label = ui->centralwidget->findChild<QLabel *>("img5");
    widget = ui->centralwidget->findChild<QWidget *>("widget");

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

    //map Vec
    for(int y = 0; y < 1440; ++y){
        for(int x = 0; x < 1440; ++x){
            //正方形(x,y)应该对应到圆形(x,y)计算像素值
            myPoint p;
            p.x = x - 720;
            p.y = 720 - y;
            QPoint ret = p2p(Sph2Rec(p, 1440));
            int tx = ret.x() + 720;
            int ty = -ret.y() + 720;
            MapVec[y][x] = QPoint(tx, ty);
        }
    }

    QVBoxLayout *layout = new QVBoxLayout();
    //openGL
    openGLWidget = new myOpenGLWidget();
    //openGLWidget->setGeometry(0,0,500,500);
    layout->addWidget(openGLWidget);
    widget->setLayout(layout);
    //openGLWidget->show();

    //on_openBtn_clicked();
    //open stream button clicked
    socketHostIpAddr = ipAddr->text();

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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openBtn_clicked()
{
    int width = 2880;
    int height = 1440;
    width = width / 2;  //正方形边长
    height = height / 2;    //正方形边长的一半
    //展开拼接,插值填充
    img3 = img1;
    img4 = img2;
    fullImage = originImage;

    //test cylinderOn
    //cylinderOn(QImage2cvMat(fullImage));

    int pattern = ipAddr->text().toInt();
    if(pattern == 1){
        cout << "直接插值，比例拉伸" << endl;
        //直接插值，比例拉伸
        for(int y = 0; y < width; ++y){
            for(int x = 0; x < width; ++x){
                //正方形(x,y)应该对应到圆形(x,y)计算像素值
                QPoint ret = MapVec[y][x];
                int retx = ret.x();
                int rety = ret.y();
                QRgb rgb1 = img1.pixel(retx, rety);
                QRgb rgb2 = img2.pixel(retx, rety);
                img3.setPixel(x, y, rgb1);
                img4.setPixel(x, y, rgb2);
                fullImage.setPixel(x, y, rgb1);
                fullImage.setPixel(x + 1440, y, rgb2);
                //cout << "point map:" << x <<"," << y << " to "<< retx << "," << rety << endl;
            }
        }
    }
    else if(pattern == 2){
        //不做处理，直接贴图旋转后拼接的图像
        cout << "不做处理" << endl;
        for(int y = 0; y < width; ++y){
            for(int x = 0; x < width; ++x){
                //正方形(x,y)应该对应到圆形(x,y)计算像素值
                QRgb rgb1 = img1.pixel(x, y);
                QRgb rgb2 = img2.pixel(x, y);
                img3.setPixel(x, y, rgb1);
                img4.setPixel(x, y, rgb2);
                fullImage.setPixel(x, y, rgb1);
                fullImage.setPixel(x + 1440, y, rgb2);
                //cout << "point map:" << x <<"," << y << " to "<< retx << "," << rety << endl;
            }
        }
    }
    else if(pattern == 3){
        cout << "经纬度展开" << endl;
        //针对3和4进行经纬度展开
        QImage temp1 = Mat2QImage(FishEyeSpread(QImage2cvMat(img3)));
        //对temp1进行剪裁
        img3 = temp1.copy(720, 0, 1440, 1440);

        QImage temp2 = Mat2QImage(FishEyeSpread(QImage2cvMat(img4)));
        //对temp2进行剪裁
        img4 = temp2.copy(720, 0, 1440, 1440);

        //rotate
        QMatrix matrix1, matrix2;
        matrix1.rotate(90.0);
        matrix2.rotate(90.0);
        img3 = img3.transformed(matrix1,Qt::FastTransformation);
        img4 = img4.transformed(matrix2,Qt::FastTransformation);

        //拼接
        for(int y = 0; y < width; ++y){
            for(int x = 0; x < width; ++x){
                //正方形(x,y)应该对应到圆形(x,y)计算像素值
                QRgb rgb1 = img3.pixel(x, y);
                QRgb rgb2 = img4.pixel(x, y);
                fullImage.setPixel(x, y, rgb1);
                fullImage.setPixel(x + 1440, y, rgb2);
                //cout << "point map:" << x <<"," << y << " to "<< retx << "," << rety << endl;
            }
        }
    }
    else{
        //else
    }

    //显示
    img3Label->setPixmap(QPixmap::fromImage(img3.scaled(img3Label->size().width(), img3Label->size().height())));
    img4Label->setPixmap(QPixmap::fromImage(img4.scaled(img4Label->size().width(), img4Label->size().height())));
    img5Label->setPixmap(QPixmap::fromImage(fullImage.scaled(img5Label->size().width(), img5Label->size().height())));
}

void MainWindow::showStream(QImage image){
    frameCount += 1;
    originImage = image;
    //cout << "get frame, frame size: " << image.size().width() << "*" << image.size().height() << endl;
    //insta view size: 2880 * 1440
    //pix=QPixmap::fromImage(image.scaled(image.width(),image.height()));
    //pix = pix.scaled(imageLabel->size().width(), imageLabel->size().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(QPixmap::fromImage(image.scaled(imageLabel->size().width(), imageLabel->size().height())));
    textureGenerate(image);
    img1Label->setPixmap(QPixmap::fromImage(img1.scaled(img1Label->size().width(), img1Label->size().height())));
    img2Label->setPixmap(QPixmap::fromImage(img2.scaled(img2Label->size().width(), img2Label->size().height())));
    //img2Label->setPixmap(QPixmap::fromImage(fullImage.scaled(img2Label->size().width(), img2Label->size().height())));
}


//根据image生成对应两个纹理图片
void MainWindow::textureGenerate(QImage image){
    //首先分离左右球面,并旋转（获得两个正方形的图片）
    int width = image.width();
    int height = image.height();
    img1 = image.copy(0, 0, width / 2, height);
    img2 = image.copy(width / 2, 0, width / 2, height);
    //rotate
    QMatrix matrix1, matrix2;
    matrix1.rotate(-90.0);
    matrix2.rotate(90.0);
    img1 = img1.transformed(matrix1,Qt::FastTransformation);
    img2 = img2.transformed(matrix2,Qt::FastTransformation);
}

void MainWindow::timer_timeout(){
    //fps
    int fps = frameCount / 5;
    frameCount = 0;
    fpsText->setText(QString::number(fps));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{

}
