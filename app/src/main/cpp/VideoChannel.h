//
// Created by 吴磊 on 4/4/21.
//

#ifndef LIVEDEMO_VIDEOCHANNEL_H
#define LIVEDEMO_VIDEOCHANNEL_H

#include <jni.h>
#include <x264.h>
#include "librtmp/rtmp.h"


class VideoChannel {
    typedef void (*VideoCallback)(RTMPPacket *packet);

public:
    VideoChannel();

    ~VideoChannel();

    //创建x264编码器
    void setVideoEncodeInfo(int width, int height, int fps, int bitrate);

    void encodeData(int8_t *data);

    void sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len);

    void sendFrame(int type, int payload, uint8_t *p_payload);

    void setVideoCallback(VideoCallback callback);

private:
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;

    x264_picture_t *pic_in = 0;

    int ySize;
    int uvSize;

    x264_t *videoCodec = 0;
    VideoCallback callback;
};


#endif //LIVEDEMO_VIDEOCHANNEL_H
