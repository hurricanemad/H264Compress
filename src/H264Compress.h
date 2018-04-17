#ifndef H264_COMPRESS_H
#define H264_COMPRESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C"{
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
}

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
using namespace std;


class H264Compress{
public:
    H264Compress(){}
    H264Compress(const char* /*pcpcFileName*/, int /*nFrameWidth*/, int /*nFrameHeight*/);
    void Compress(cv::Mat& /*matSrcMat*/,int /*nFrameCount*/);
    ~H264Compress();
    
private:
    void Initialize(const char* /*pcpcFileName*/, int /*nFrameWidth*/, int /*nFrameHeight*/);
    void ConvertFrame(cv::Mat& /*matCameraFrame*//*, AVFrame* pavFrame*/);
    void encode(AVFrame */*pavFrame*/);
    
    const char *m_cFileName, *m_cOutFileName;
    const AVCodec *m_pavCodec;
    AVCodecParserContext *m_pavCodecPC;
    AVCodecContext *m_pavCodecContext;
    FILE* m_pfDecode;
    AVFrame *m_pavFrame;
    uint8_t* m_puinbuf;
    uint8_t* m_puData;
    size_t m_sztDataSize;
    int m_nResult;
    AVPacket *m_pavPkt;
    int m_nFrameWidth;
    int m_nFrameHeight;
    int m_nFrameCount;
};

#endif
