#include <QImage>
#include <QRgb>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

//mat 2 QImage
QImage Mat2QImage(const cv::Mat& InputMat)
{
    cv::Mat TmpMat;
    // convert the color space to RGB
    if (InputMat.channels() == 1){
        cv::cvtColor(InputMat, TmpMat, CV_GRAY2RGB);
    }
    else{
        cv::cvtColor(InputMat, TmpMat, CV_BGR2RGB);
    }
    // construct the QImage using the data of the mat, while do not copy the data
    QImage Result = QImage((const uchar*)(TmpMat.data), TmpMat.cols, TmpMat.rows, QImage::Format_RGB888);
    Result.convertTo(QImage::Format_ARGB32);
    // deep copy the data from mat to QImage
    Result.bits();
    return Result;
}

//QImage 2 mat
cv::Mat QImage2cvMat(QImage image)
{
    //image.convertTo(QImage::Format_RGB888);
//    QImage image = QImage(1440,1440,QImage::Format_RGB888);
//    int width = 1440;
//    for(int y = 0; y < width; ++y){
//        QRgb * line = (QRgb *)src.scanLine(y);
//        for(int x = 0; x < width; x++){
//            image.setPixel(x,y, (qRgb(qBlue(line[x]), qGreen(line[x]), qRed(line[x])) & 0xffffff));
//        }
//    }
    cv::Mat mat;
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

