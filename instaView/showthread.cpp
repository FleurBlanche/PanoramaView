#include "showthread.h"
#include <iostream>
#include <windows.h>
#include <QTranslator>


using namespace std;

int sum = 0;
int sumNet = 0;
int times = 0;
int got_picture = 0;
QImage showImage;
bool getFrame = 0;

extern QString socketHostIpAddr;

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

int interrupt(void *ctx)
{
    bool *r = (bool *)ctx;
    if(*r)
    {
        return AVERROR_EOF;
    }
    return 0;
}

showThread::showThread()
{
    stopBit = true;
    takePic = false;
    exitDecode = false;
    frameCounts = 0;
    prevCounts = 0;

}

void showThread::stopBroad()
{
    stopBit = false;
}

void showThread::setExit(bool c)
{
    exitDecode = c;
}



void showThread::setPic(bool c)
{
    takePic = c;
}

void showThread::run()
{
    AVFormatContext *pFormatCtx;
    int             i, videoindex;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    string filepathO = "rtmp://" + socketHostIpAddr.toStdString();

    const char* filepath= (filepathO + ":1935/live/live").c_str();
    //rtmp://192.168.2.105:1935/testLive/livestream
    //rtsp://192.168.1.123:8554/1
    cout << filepath << endl;

    AVDictionary* opts = NULL;
    //av_dict_set(&opts, "timeout", "10000000", 0); // 设置timeout，为微秒。一共5秒
    av_dict_set(&opts, "buffer_size", "102400", 0);
    //av_dict_set(&opts,"framerate","5",0);

    while(stopBit){
        exitDecode = false;
        pFormatCtx = avformat_alloc_context();
        pFormatCtx->interrupt_callback.callback = interrupt;
        pFormatCtx->interrupt_callback.opaque = &exitDecode;
        cout << "retrying" << endl;
        if(avformat_open_input(&pFormatCtx,filepath,0,&opts)!=0){
            cout << "Couldn't open input stream.\n" << endl;
            avformat_close_input(&pFormatCtx);
            avformat_free_context(pFormatCtx);
            pFormatCtx = NULL;
            sleep(1);
            continue;
        }

        //cout << "hey baby" << endl;
        pFormatCtx->max_analyze_duration = 100000;
        if(avformat_find_stream_info(pFormatCtx,NULL)<0)
        {
            printf("Couldn't find stream information.\n");
            continue;
        }
        videoindex=-1;
        for(i=0; i< pFormatCtx->nb_streams; i++)
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
            {
                videoindex=i;
                break;
            }
        if(videoindex==-1)
        {
            printf("Didn't find a video stream.\n");
            continue;
        }
        pCodecCtx=pFormatCtx->streams[videoindex]->codec;
        pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec==NULL)
        {
            printf("Codec not found.\n");
            continue;
        }
        pCodecCtx->thread_count = 5;
        if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
        {
            printf("Could not open codec.\n");
            continue;
        }
        AVFrame *pFrame,*pFrameYUV,*pFrameRGB;
        pFrame=av_frame_alloc();
        pFrameYUV=av_frame_alloc();
        pFrameRGB=av_frame_alloc();

        unsigned char *out_buffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
        unsigned char *rgbBuffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height));

        avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);//以上就是为pFrameRGB挂上buffer。这个buffer是用于存缓冲数据的


        //av_dump_format(pFormatCtx, 0, filepath, 0);
        int ret, got_picture;

        AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));


        struct SwsContext *img_convert_ctx,*img_convert_ctx_rgb;
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

        //需要注意的地方
        avpicture_fill((AVPicture *)pFrameRGB,rgbBuffer,AV_PIX_FMT_RGB32,pCodecCtx->width, pCodecCtx->height);
        img_convert_ctx_rgb=sws_getContext(pCodecCtx->width,pCodecCtx->height,AV_PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height,AV_PIX_FMT_RGB32,SWS_BICUBIC, NULL, NULL, NULL);

        while(((av_read_frame(pFormatCtx, packet))>=0) && stopBit){
            if(packet->stream_index==videoindex)
            {
                ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);//用packet充填pFrame,pFrame->data = packet->data?  pFrame->linesize=packet->linesize
                if(ret < 0){
                    printf("Decode Error.\n");
                }
                if(got_picture)
                {
                    frameCounts++;
                    if(img_convert_ctx_rgb != NULL)
                    {
                        //转码为rgb32
                        sws_scale(img_convert_ctx_rgb,pFrame->data,pFrame->linesize,0,pCodecCtx->height,pFrameRGB->data,pFrameRGB->linesize);
                        //构造QImage，用于主页面显示
                        QImage image((uchar *)pFrameRGB->data[0],pCodecCtx->width, pCodecCtx->height,QImage::Format_ARGB32);
                        emit sendImage(image);
                    }
                }
            }
            av_free_packet(packet);
        }

        if(img_convert_ctx)
            sws_freeContext(img_convert_ctx);
        if(img_convert_ctx_rgb)
            sws_freeContext(img_convert_ctx_rgb);
        av_free(rgbBuffer);
        av_free(out_buffer);
        av_frame_free(&pFrame);
        av_frame_free(&pFrameYUV);
        av_frame_free(&pFrameRGB);
        av_packet_unref(packet);
        av_packet_free(&packet);
        avcodec_close(pCodecCtx);
        if(pFormatCtx != NULL){
            avformat_close_input(&pFormatCtx);
            avformat_free_context(pFormatCtx);
            pFormatCtx = NULL;
        }

    }
    cout << "broadexit" << endl;
}
