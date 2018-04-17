#include "prefix.h"

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
                   FILE *outfile)
{
    int ret;
    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3"PRId64"\n", frame->pts);
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}

void ConvertFrame(Mat& matCameraFrame, AVFrame* pavFrame){
    int nMatWidth = matCameraFrame.cols;
    int nMatHeight = 2*matCameraFrame.rows/3;
    

    int nLineSize = pavFrame->linesize[0];
    cout << "nLineSize:" << nLineSize <<endl;
    cout << "nMatWidth:" << nMatWidth <<endl;
    cout << "nMatHeight:" << nMatHeight <<endl;
    cout << "pavFrame->linesize[1]:" << pavFrame->linesize[1] <<endl;
    cout << "pavFrame->linesize[2]:" << pavFrame->linesize[2] <<endl;
    
    int r, c;
    uchar* pmatCameraFrame;
    uchar* pmatavFrame0 = (uchar*)(pavFrame->data[0]);
    for(r=0; r< nMatHeight; r++){
        pmatCameraFrame = matCameraFrame.ptr<uchar>(r);
        for(c=0; c < nLineSize; c++){
            if(c < nMatWidth){
                pmatavFrame0[r * pavFrame->linesize[0] + c] = pmatCameraFrame[c];
            }else{
                pmatavFrame0[r * pavFrame->linesize[0] + c] = 0;
            }
        }
    }
    
    uchar* pmatavFrame1 = (uchar*)(pavFrame->data[1]);
    int nTempR, nTempC, ntempc;
    int n;
    for (r = 0; r < nMatHeight/4; r++) {
        nTempR = 2*r;
        pmatCameraFrame = matCameraFrame.ptr<uchar>(r + nMatHeight);
        for(n=0; n < 2; n++){
            nTempC = n * matCameraFrame.cols/2;
            for (c = 0; c < pavFrame->linesize[1]; c++) {
                ntempc = c + nTempC;
                if(c < matCameraFrame.cols/2){
                    pmatavFrame1[r * pavFrame->linesize[1] + c] = pmatCameraFrame[ntempc];
                }else{
                    pmatavFrame1[r * pavFrame->linesize[1] + c] = 0;
                }
            }
        }
    }
    uchar* pmatavFrame2 = (uchar*)(pavFrame->data[2]);
    for (r = 0; r < nMatHeight/4; r++) {
        pmatCameraFrame = matCameraFrame.ptr<uchar>(r + 5 * nMatHeight/4);
        for(n=0; n < 2; n++){
            nTempC = n * matCameraFrame.cols/2;
            for (c = 0; c < pavFrame->linesize[2]; c++) {
                ntempc = c + nTempC;
                if(c < matCameraFrame.cols/2){
                    pmatavFrame2[r * pavFrame->linesize[2] + c] = pmatCameraFrame[ntempc];
                }else{
                    pmatavFrame2[r * pavFrame->linesize[2] + c] = 0;
                }
            }
        }
    }
}
 
int main(int argc, char **argv) {
    VideoCapture vpCamera(0);
    if(!vpCamera.isOpened()){
        printf("Camera doesn't open!");
        exit(-1);
    }
    
    double dFPS=vpCamera.get(CV_CAP_PROP_FPS);
    vpCamera.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    vpCamera.set(CV_CAP_PROP_FRAME_HEIGHT, 360);
    int nFrameWidth, nFrameHeight;
    nFrameWidth = vpCamera.get(CV_CAP_PROP_FRAME_WIDTH);
    nFrameHeight = vpCamera.get(CV_CAP_PROP_FRAME_HEIGHT);
    
    H264Compress h264compress("camera.h264", 640, 360);
    int nFrameCount = 0;
    int nWaitKey = 0;
    Mat matCameraFrame;
    /* encode 1 second of video */
    while(27 != nWaitKey && nFrameCount < 100){

        fflush(stdout);


        vpCamera >> matCameraFrame;

        h264compress.Compress(matCameraFrame, nFrameCount);
        nFrameCount++;

        namedWindow("EncodeFrame");
        imshow("EncodeFrame", matCameraFrame);
        nWaitKey = waitKey(1);
    }

    return 0;
}
