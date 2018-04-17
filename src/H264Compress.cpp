#include "H264Compress.h"

H264Compress::H264Compress(const char* pcpcFileName, int nFrameWidth, int nFrameHeight){
    Initialize(pcpcFileName, nFrameWidth, nFrameHeight);
}

void H264Compress::Initialize(const char* pcpcFileName, int nFrameWidth, int nFrameHeight)
{
    m_nFrameWidth = nFrameWidth;
    m_nFrameHeight = nFrameHeight;
    m_cOutFileName = pcpcFileName;
    
    avcodec_register_all();
    m_pavCodec = avcodec_find_encoder_by_name("libx264");
    if (!m_pavCodec) {
        fprintf(stderr, "Codec libx264 not found\n");
        exit(1);
    }
    
    m_pavCodecContext = avcodec_alloc_context3(m_pavCodec);
    if (!m_pavCodecContext) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    
    m_pavPkt = av_packet_alloc();
    if (!m_pavPkt)
        exit(1);
    /* put sample parameters */
    m_pavCodecContext->bit_rate = 1000000;
    /* resolution must be a multiple of two */
    m_pavCodecContext->width = nFrameWidth;
    m_pavCodecContext->height = nFrameHeight;
    /* frames per second */
    m_pavCodecContext->time_base = (AVRational){1, 25};
    m_pavCodecContext->framerate = (AVRational){25, 1};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    m_pavCodecContext->gop_size = 10;
    m_pavCodecContext->max_b_frames = 1;
    m_pavCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if (m_pavCodec->id == AV_CODEC_ID_H264)
        av_opt_set(m_pavCodecContext->priv_data, "preset", "slow", 0);
    /* open it */
    m_nResult = avcodec_open2(m_pavCodecContext, m_pavCodec, NULL);
    if (m_nResult < 0) {
        fprintf(stderr, "Could not open codec!\n");
        exit(1);
    }
    m_pfDecode = fopen(pcpcFileName, "wb+");
    if (!m_pfDecode) {
        fprintf(stderr, "Could not open %s\n", pcpcFileName);
        exit(1);
    }
    m_pavFrame = av_frame_alloc();
    if (!m_pavFrame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    m_pavFrame->format = m_pavCodecContext->pix_fmt;
    m_pavFrame->width  = m_pavCodecContext->width;
    m_pavFrame->height = m_pavCodecContext->height;
    m_nResult = av_frame_get_buffer(m_pavFrame, 32);
    if (m_nResult < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }
    
}

void H264Compress::Compress(cv::Mat& matSrcMat, int nFrameCount){
    /* make sure the frame data is writable */
    m_nResult = av_frame_make_writable(m_pavFrame);
    double dCurentT = static_cast<double>(cv::getTickCount());        
    if (m_nResult < 0)
        exit(1);
    
    cv::Mat matYUVFrame;
    cvtColor(matSrcMat, matYUVFrame, cv::COLOR_RGB2YUV_YV12);
    ConvertFrame(matYUVFrame/*, m_pavFrame*/);
    m_nFrameCount = nFrameCount;
    m_pavFrame->pts = nFrameCount;
    encode(m_pavFrame);
    double dProcessTime = (static_cast<double>(cv::getTickCount()) - dCurentT)/ cv::getTickFrequency();
    cout << "Compress time is:" << dProcessTime <<endl;
}

void H264Compress::encode(AVFrame *pavFrame){
    /* send the frame to the encoder */
    if (pavFrame)
        printf("Send frame %3"PRId64"\n", m_pavFrame->pts);
    m_nResult = avcodec_send_frame(m_pavCodecContext, pavFrame);
    if (m_nResult < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }
    while (m_nResult >= 0) {
        m_nResult = avcodec_receive_packet(m_pavCodecContext, m_pavPkt);
        if (m_nResult == AVERROR(EAGAIN) || m_nResult == AVERROR_EOF)
            return;
        else if (m_nResult < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        printf("Write packet %3"PRId64" (size=%5d)\n", m_pavPkt->pts, m_pavPkt->size);
        fwrite(m_pavPkt->data, 1, m_pavPkt->size, m_pfDecode);
        av_packet_unref(m_pavPkt);
    }
}

void H264Compress::ConvertFrame(cv::Mat& matYUVFrame/*, AVFrame* pavFrame*/){
    //int nMatWidth = matYUVFrame.cols;
    //int nMatHeight = 2*matYUVFrame.rows/3;
    

    int nLineSize = m_pavFrame->linesize[0];
    cout << "nLineSize:" << nLineSize <<endl;
    //cout << "nMatWidth:" << nMatWidth <<endl;
    //cout << "nMatHeight:" << nMatHeight <<endl;
    cout << "pavFrame->linesize[1]:" << m_pavFrame->linesize[1] <<endl;
    cout << "pavFrame->linesize[2]:" << m_pavFrame->linesize[2] <<endl;
    
    int r, c;
    uchar* pmatYUVFrame;
    uchar* pmatavFrame0 = (uchar*)(m_pavFrame->data[0]);
    for(r=0; r< m_nFrameHeight; r++){
        pmatYUVFrame = matYUVFrame.ptr<uchar>(r);
        for(c=0; c < nLineSize; c++){
            if(c < m_nFrameWidth){
                pmatavFrame0[r * m_pavFrame->linesize[0] + c] = pmatYUVFrame[c];
            }else{
                pmatavFrame0[r * m_pavFrame->linesize[0] + c] = 0;
            }
        }
    }
    
    uchar* pmatavFrame1 = (uchar*)(m_pavFrame->data[1]);
    int nTempR, nTempC, ntempc;
    int n;
    for (r = 0; r < m_nFrameHeight/4; r++) {
        nTempR = 2*r;
        pmatYUVFrame = matYUVFrame.ptr<uchar>(r + m_nFrameHeight);
        for(n=0; n < 2; n++){
            nTempC = n * m_nFrameWidth/2;
            for (c = 0; c < m_pavFrame->linesize[1]; c++) {
                ntempc = c + nTempC;
                if(c < m_nFrameWidth/2){
                    pmatavFrame1[r * m_pavFrame->linesize[1] + c] = pmatYUVFrame[ntempc];
                }else{
                    pmatavFrame1[r * m_pavFrame->linesize[1] + c] = 0;
                }
            }
        }
    }
    uchar* pmatavFrame2 = (uchar*)(m_pavFrame->data[2]);
    for (r = 0; r < m_nFrameHeight/4; r++) {
        pmatYUVFrame = matYUVFrame.ptr<uchar>(r + 5 * m_nFrameHeight/4);
        for(n=0; n < 2; n++){
            nTempC = n * m_nFrameWidth/2;
            for (c = 0; c < m_pavFrame->linesize[2]; c++) {
                ntempc = c + nTempC;
                if(c < m_nFrameWidth/2){
                    pmatavFrame2[r * m_pavFrame->linesize[2] + c] = pmatYUVFrame[ntempc];
                }else{
                    pmatavFrame2[r * m_pavFrame->linesize[2] + c] = 0;
                }
            }
        }
    }
}

H264Compress::~H264Compress(){
    encode(NULL);
    fclose(m_pfDecode);
    avcodec_free_context(&m_pavCodecContext);
    av_frame_free(&m_pavFrame);
    av_packet_free(&m_pavPkt);
}
