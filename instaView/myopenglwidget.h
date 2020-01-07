#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QObject>
#include <QWidget>
#include <QOpenGLWidget>
#include <QtOpenGL>

#include "freeglut.h"

class myOpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    myOpenGLWidget();

protected:
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    GLUquadricObj *sphere;

    bool projection = false;
    bool isEarth = true;
    bool isInside = true;
};

#endif // MYOPENGLWIDGET_H
