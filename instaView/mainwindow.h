#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLineEdit>
#include <QStatusBar>
#include <QMenuBar>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QTimer>
#include "showthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    showThread *videoStream = nullptr;
    QPixmap pix;

    QOpenGLWidget *openGLWidget;

    QLineEdit *ipAddr;
    QPushButton *openBtn;
    QMenuBar *menuBar;
    QStatusBar *statusBar;
    QLabel *imageLabel;
    QLabel *img1Label;
    QLabel *img2Label;
    QLabel *img3Label;
    QLabel *img4Label;
    QLabel *img5Label;

    QWidget *widget;

    QTextEdit *fpsText;

    int timeOut = 5000;
    int frameCount = 0;

    void textureGenerate(QImage image);

    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void showStream(QImage image);
    void on_openBtn_clicked();
    void timer_timeout();
};
#endif // MAINWINDOW_H
