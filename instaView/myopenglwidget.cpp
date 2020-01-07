#include "myopenglwidget.h"
#include <QObject>
#include <QWidget>
#include <QOpenGLWidget>
#include <QtOpenGL>
#include <iostream>
#include <QImage>

#define _STDCALL_SUPPORTED
#define GLUT_DISABLE_ATEXIT_HACK
#include "freeglut.h"

using namespace std;

//variables
#define PI 3.1415926535

//纹理
extern QImage fullImage;
extern QImage originImage;
extern QImage earth;
QImage texImage;
GLuint texture[2];

//球面 等面积投影
int WORLDSIZE = 1000;

float c = PI / 180.0f;
int du = 90, oldmy = 0, oldmx = 0;
float r = WORLDSIZE;
float h = 0.0f;

//sphere
int RADIUS = 509;   //等面积投影
float *verticals;
float *UV_TEX_VERTEX;
int cap_H = 1;
int cap_W = 1;

//get sphere point
void getPointMatrix()
{
    verticals = new float[(180 / cap_H) * (360 / cap_W) * 6 * 3];
    UV_TEX_VERTEX = new float[(180 / cap_H) * (360 / cap_W) * 6 * 2];
    float x = 0;
    float y = 0;
    float z = 0;
    int index = 0;
    int index1 = 0;
    float r = RADIUS;//球体半径
    double d = cap_H * c;//每次递增的弧度
    for (int i = 0; i < 180; i += cap_H) {
        double d1 = i * c;
        for (int j = 0; j < 360; j += cap_W) {
            //获得球体上切分的超小片矩形的顶点坐标（两个三角形组成，所以有六点顶点）
            double d2 = j * c;
            verticals[index++] = (float)(x + r * sin(d1 + d) * cos(d2 + d));
            verticals[index++] = (float)(y + r * cos(d1 + d));
            verticals[index++] = (float)(z + r * sin(d1 + d) * sin(d2 + d));
            //获得球体上切分的超小片三角形的纹理坐标
            UV_TEX_VERTEX[index1++] = (j + cap_W) * 1.0f / 360;
            UV_TEX_VERTEX[index1++] = (i + cap_H) * 1.0f / 180;

            verticals[index++] = (float)(x + r * sin(d1) * cos(d2));
            verticals[index++] = (float)(y + r * cos(d1));
            verticals[index++] = (float)(z + r * sin(d1) * sin(d2));

            UV_TEX_VERTEX[index1++] = j * 1.0f / 360;
            UV_TEX_VERTEX[index1++] = i * 1.0f / 180;

            verticals[index++] = (float)(x + r * sin(d1) * cos(d2 + d));
            verticals[index++] = (float)(y + r * cos(d1));
            verticals[index++] = (float)(z + r * sin(d1) * sin(d2 + d));

            UV_TEX_VERTEX[index1++] = (j + cap_W) * 1.0f / 360;
            UV_TEX_VERTEX[index1++] = i * 1.0f / 180;

            verticals[index++] = (float)(x + r * sin(d1 + d) * cos(d2 + d));
            verticals[index++] = (float)(y + r * cos(d1 + d));
            verticals[index++] = (float)(z + r * sin(d1 + d) * sin(d2 + d));

            UV_TEX_VERTEX[index1++] = (j + cap_W) * 1.0f / 360;
            UV_TEX_VERTEX[index1++] = (i + cap_H) * 1.0f / 180;

            verticals[index++] = (float)(x + r * sin(d1 + d) * cos(d2));
            verticals[index++] = (float)(y + r * cos(d1 + d));
            verticals[index++] = (float)(z + r * sin(d1 + d) * sin(d2));

            UV_TEX_VERTEX[index1++] = j * 1.0f / 360;
            UV_TEX_VERTEX[index1++] = (i + cap_H) * 1.0f / 180;

            verticals[index++] = (float)(x + r * sin(d1) * cos(d2));
            verticals[index++] = (float)(y + r * cos(d1));
            verticals[index++] = (float)(z + r * sin(d1) * sin(d2));

            UV_TEX_VERTEX[index1++] = j * 1.0f / 360;
            UV_TEX_VERTEX[index1++] = i * 1.0f / 180;
        }
    }
}

myOpenGLWidget::myOpenGLWidget()
{
    this->setFocusPolicy(Qt::StrongFocus);
    //init
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    //sphere init
    sphere = gluNewQuadric();
    //line | fill
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricNormals(sphere,GL_SMOOTH);
    gluQuadricTexture(sphere,TRUE);
}

void myOpenGLWidget::paintGL()
{
    makeCurrent();

    //显示函数
    //GL_COLOR_BUFFER_BIT(用背景颜色填充)4
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(projection){
        glMatrixMode(GL_PROJECTION);//选择投影矩阵
        glLoadIdentity();//重置选择好的投影矩阵
        glOrtho(-WORLDSIZE * 0.5, WORLDSIZE * 0.5, -WORLDSIZE * 0.5, WORLDSIZE * 0.5, 0, WORLDSIZE);
        glMatrixMode(GL_MODELVIEW);
    }
    else{
        int w = this->width();
        int h = this->height();
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);//选择投影矩阵
        glLoadIdentity();//重置选择好的投影矩阵
        gluPerspective(100.0f, (float)w / h, 1.0f, 2000.0f);
        glMatrixMode(GL_MODELVIEW);
    }

    glLoadIdentity();

    //调整视点
    if(isInside){
        gluLookAt(0, 0, 0, r * cos(c * du) / 50.0, h / 50.0, r * sin(c * du) / 50.0, 0, 1, 0);
    }
    else{
        gluLookAt(r * cos(c * du) * 1.5 , h / 3.0, r * sin(c * du) * 1.5 , 0, 0, 0, 0, 1, 0);
    }
    //gluLookAt(r * cos(c * du) * 1.2, h * 1.5, r * sin(c * du) * 1.2, 0, 0, 0, 0, 1, 0);

    //坐标轴以及测试平面
//    //pixels
//    glColor3f(1, 0, 0); // ------x  red
//    glBegin(GL_LINE_STRIP);
//    glVertex3f(-1000, 0, 0);
//    glVertex3f(0, 0, 0);
//    glVertex3f(1000, 0, 0);
//    glEnd();

//    glColor3f(0, 0, 1); // --------y blue
//    glBegin(GL_LINE_STRIP);
//    glVertex3f(0, -1000, 0);
//    glVertex3f(0, 0, 0);
//    glVertex3f(0, 1000, 0);
//    glEnd();

//    glColor3f(0, 1, 0); //--------z  green
//    glBegin(GL_LINE_STRIP);
//    glVertex3f(0, 0, -1000);
//    glVertex3f(0, 0, 0);
//    glVertex3f(0, 0, 1000);
//    glEnd();

//    glColor3f(0.5, 1, 0.5);
//    glPushMatrix();
//    glTranslatef(100.0f, 0, 0);
//    glRotatef(270, 0, 1, 0);
//    glBegin(GL_QUADS);
//    glVertex3f(-10.0f, 10.0f, 10.0f); // 左上顶点
//    glVertex3f( 10.0f, 10.0f, 10.0f); // 右上顶点
//    glVertex3f( 10.0f, -10.0f, 10.0f); // 右下顶点
//    glVertex3f(-10.0f, -10.0f, 10.0f); // 左下顶点
//    glEnd(); // 四边形绘制结束
//    glPopMatrix();

    glPushMatrix();
    //旋转，调整图像
    glRotatef(90,1,0,0);
    glRotatef(180,0,1,0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //纹理获取与绑定

    if(isEarth){
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
        glBindTexture(GL_TEXTURE_2D,texture[1]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexImage2D(GL_TEXTURE_2D,0,3,earth.width(),earth.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,earth.bits());
    }
    else{
        texImage = QGLWidget::convertToGLFormat(fullImage);
        //texImage = QGLWidget::convertToGLFormat(originImage);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
        glBindTexture(GL_TEXTURE_2D,texture[0]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexImage2D(GL_TEXTURE_2D,0,3,texImage.width(),texImage.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texImage.bits());
        //glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,texImage.width(),texImage.height(),0,GL_RGB,GL_UNSIGNED_BYTE,texImage.bits());
    }

    //二次曲面绘制
    gluSphere(sphere, WORLDSIZE, 30, 30);

    glPopMatrix();
    doneCurrent();
    glFlush();
}

void myOpenGLWidget::initializeGL()
{
    //界面初始化函数
    glClearColor(1, 1, 1, 1.0); //初始背景色 ，白色
    /******** 光照处理 **********/
    GLfloat light_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position0[] = { 0.0, 0.0, 0.0 ,1 };
    /******** 光照定义 ***********/
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient); //环境光,白色
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse); //漫射光
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular); //镜面反射
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0); //光照位置
    /******** 材质处理 ***********/
    GLfloat mat_ambient[] = { 0.0, 0.2, 1.0, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.5, 0.2, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 100.0 };
    //glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    //glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    //glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glEnable(GL_LIGHTING); //启动光照
    glEnable(GL_LIGHT0);

    glEnable(GL_DEPTH_TEST); //深度缓存

    //getPointMatrix();   //获取球面坐标点

    glGenTextures(1, &texture[0]);  //生成纹理
    glEnable(GL_TEXTURE_2D);

    //earth texture
    glGenTextures(1, &texture[1]);
    earth = QGLWidget::convertToGLFormat(earth);
    //texImage = QGLWidget::convertToGLFormat(originImage);
}

void myOpenGLWidget::resizeGL(int w, int h)
{
    //大小变化函数
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);//选择投影矩阵
    glLoadIdentity();//重置选择好的投影矩阵
    gluPerspective(100.0f, (float)w / h, 1.0f, 2000.0f);
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
    h -= 20.0f * ym;

    //cout << "du: " << du << "h: " << h << endl;

    if (h > WORLDSIZE * 3) {
        h = WORLDSIZE * 3;
    }
    else if (h < -WORLDSIZE * 3) {
        h = -WORLDSIZE * 3;
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

void myOpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    int w = this->width();
    int h = this->height();
    switch(event->key()) {
    case (Qt::Key_Space):
        //空格键响应
        cout << "OPENGL project model change." << endl;
        if(projection){
            //change to 水平投影
            projection = false;
            //大小变化函数
            glViewport(0, 0, w, h);
            glMatrixMode(GL_PROJECTION);//选择投影矩阵
            glLoadIdentity();//重置选择好的投影矩阵
            glOrtho(-200,200,-200,200,0,WORLDSIZE);
            glMatrixMode(GL_MODELVIEW);
        }
        else{
            //change to 透视投影
            projection = true;
            //大小变化函数
            glViewport(0, 0, w, h);
            glMatrixMode(GL_PROJECTION);//选择投影矩阵
            glLoadIdentity();//重置选择好的投影矩阵
            gluPerspective(110.0f, (float)w / h, 1.0f, 2000.0f);
            glMatrixMode(GL_MODELVIEW);
        }
        break;
    case (Qt::Key_S):
        isEarth = !isEarth;
        break;
    case (Qt::Key_Escape):
        isInside = !isInside;
        break;
    }
}
