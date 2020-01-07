#include <iostream>
#include <string>
#include <vector>
#include <QPoint>
#include <opencv2\opencv.hpp>

using namespace std;
//using namespace cv;

//绝对值比大小,|x|>|y|就返回1，否则返回0
#define COMPARE(x,y) (abs(x) > abs(y) ? 1 : 0)

//符号变换，让x变为y的相同正负
#define SETPN(x,y) (((y) > 0) ? (abs(x)) : (-abs(x)))

//平方
#define POW(x) (pow(abs(x),2))

//映射关系向量vector 1440 x 1440
vector<vector<QPoint>> MapVec(1440, vector<QPoint> (1440));

//定义浮点数数值的点结构
//会使用到像素的插值计算中
struct myPoint
{
    float x;
    float y;
};

//点对点映射,球面图像展开时的投影变换
//size默认为偶数
//数学推导详见图形学课程笔记
//需要按照比例进行采样
//比例计算结果会使用到简单插值的计算
myPoint Sph2Rec(myPoint p, int size){
    int retx = 0;
    int rety = 0;
    int mid = size / 2;
    float x0 = p.x;
    float y0 = p.y;

    //获取正方形图像中p坐标的点，
    //对应到圆形图像中坐标的的比例位置

    //坐标轴上的情况,直接返回原坐标
    if(abs(x0) <= 0.5 || abs(y0) <= 0.5){
        retx = x0;
        rety = y0;
    }
    else{
        //非坐标轴上，按比例，对原点映射
        if(COMPARE(x0,y0)){
            //x0绝对值大，向x轴投影，计算比例坐标
            retx = sqrt((POW(x0) / (POW(x0) + POW(y0))) * POW(x0));
            retx = SETPN(retx, x0);
            rety = (y0 / x0) * retx;
        }
        else{
            //y0绝对值大，向y轴投影，计算比例坐标
            retx = sqrt((POW(x0) / (POW(x0) + POW(y0))) * POW(y0));
            retx = SETPN(retx, x0);
            rety = (y0 / x0) * retx;
        }
    }
    //返回值
    myPoint ret;
    ret.x = retx;
    ret.y = rety;
    return ret;
}

//插值算法
//最近邻元法
QPoint p2p(myPoint p){
    float x = p.x;
    float y = p.y;
    int retx = (int)(x + 0.5f);
    int rety = (int)(y + 0.5f);
    QPoint ret = QPoint(retx, rety);
    return ret;
}
