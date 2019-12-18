#include "myopenglwidget.h"
#include <QObject>
#include <QWidget>
#include <QOpenGLWidget>
#include <QtOpenGL>
#include <iostream>

#define _STDCALL_SUPPORTED
#define GLUT_DISABLE_ATEXIT_HACK
#include "freeglut.h"

using namespace std;

//variables
#define PI 3.1415926535

int WORLDSIZE = 500;
int WINDOW_H = 540;
int WINDOW_W = 960;

float c = PI / 180.0f;
int du = 90, oldmy = 0, oldmx = 0;
float r = WORLDSIZE;
float h = 0.0f;

float view_x = 0;
float view_y = 0;
float view_z = 0;

myOpenGLWidget::myOpenGLWidget()
{
    //init
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

}

void myOpenGLWidget::paintGL()
{
    makeCurrent();

    //显示函数
    //GL_COLOR_BUFFER_BIT(用背景颜色填充)4
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();


    //调整视点，从零点往外看
    //gluLookAt(view_x - 25 * cos(c * du), view_y + 20.0, view_z - 25 * sin(c * du), r * cos(c * du) / 5.0, h / 5.0, r * sin(c * du) / 5.0, 0, 1, 0);

    gluLookAt(r * cos(c * du) / 5.0, h / 5.0, r * sin(c * du) / 5.0, 0, 0, 0, 0, 1, 0);

    //pixels
    glColor3f(1, 0, 0); // ------x  red
    glBegin(GL_LINE_STRIP);
    glVertex3f(-1000, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(1000, 0, 0);
    glEnd();

    glColor3f(0, 0, 1); // --------y blue
    glBegin(GL_LINE_STRIP);
    glVertex3f(0, -1000, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1000, 0);
    glEnd();

    glColor3f(0, 1, 0); //--------z  green
    glBegin(GL_LINE_STRIP);
    glVertex3f(0, 0, -1000);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1000);
    glEnd();

    glColor3f(0.5, 1, 0.5);
    glPushMatrix();
    glTranslatef(100.0f, 0, 0);
    glRotatef(270, 0, 1, 0);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, 10.0f, 10.0f); // 左上顶点
    glVertex3f( 10.0f, 10.0f, 10.0f); // 右上顶点
    glVertex3f( 10.0f, -10.0f, 10.0f); // 右下顶点
    glVertex3f(-10.0f, -10.0f, 10.0f); // 左下顶点
    glEnd(); // 四边形绘制结束
    glPopMatrix();

    //glutWireSphere(100,10,10);

    doneCurrent();

    glFlush();
    glutSwapBuffers();
}

void myOpenGLWidget::initializeGL()
{
    //界面初始化函数
    glClearColor(1, 1, 1, 1.0); //初始背景色 ，白色
    /******** 光照处理 **********/
    GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position0[] = { 2000.0, 2000.0, 2000.0 ,0 };
    /******** 光照定义 ***********/
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient); //环境光
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse); //漫射光
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular); //镜面反射
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0); //光照位置
    /******** 材质处理 ***********/
    GLfloat mat_ambient[] = { 0.0, 0.2, 1.0, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.5, 0.2, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 100.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glEnable(GL_LIGHTING); //启动光照
    glEnable(GL_LIGHT0);

    glEnable(GL_DEPTH_TEST); //深度缓存
}

void myOpenGLWidget::resizeGL(int w, int h)
{
    //大小变化函数
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);//选择投影矩阵
    glLoadIdentity();//重置选择好的投影矩阵
    gluPerspective(120.0f, (float)w / h, 1.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void myOpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->x();
    int y = event->y();

    //cout << "detect mouse move from (" << oldmx << "," << oldmy << ") to (" << x << "," << y  << ")"<< endl;
    int xm = x - oldmx;
    int ym = y - oldmy;
    //cout << xm << "," << ym << endl;

    //mouse move
    du += xm;
    h -= 15.0f * ym;

    //cout << "du: " << du << "h: " << h << endl;

    if (h > WORLDSIZE * 2) {
        h = WORLDSIZE * 2;
    }
    else if (h < -WORLDSIZE * 2) {
        h = -WORLDSIZE * 2;
    }
    oldmx = x;
    oldmy = y;

    this->update();
}

void myOpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    int x = event->x();
    int y = event->y();

    //cout << "detect mouse press: " << x << ", " << y << endl;

    //mouse press
    if (1) {
        oldmx = x;
        oldmy = y;
    }

    this->update();
}
