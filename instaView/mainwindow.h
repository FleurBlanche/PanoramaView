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

    QTextEdit *fpsText;

    int timeOut = 1000;
    int frameCount = 0;

private slots:
    void showStream(QImage image);
    void on_openBtn_clicked();
    void timer_timeout();
};
#endif // MAINWINDOW_H
