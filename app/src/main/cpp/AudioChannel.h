//
// Created by 吴磊 on 4/4/21.
//

#ifndef LIVEDEMO_AUDIOCHANNEL_H
#define LIVEDEMO_AUDIOCHANNEL_H

#include <faac.h>
#include "librtmp/rtmp.h"


typedef void (*AudioCallback)(RTMPPacket *packet);

class AudioChannel {

public:
    AudioChannel();

    ~AudioChannel();

    void openCodec(int sampleRate, int channels);

    //编码函数
    void encode(int32_t *data, int len);

    //头帧
    RTMPPacket *getAudioConfig();

    void setCallback(AudioCallback callback) {
        this->callback = callback;
    }

    int getInputByteNum() {
        return inputByteNum;
    }

private:
    AudioCallback callback;
    faacEncHandle codec = 0;

    //音频压缩成aac后最大数据量
    unsigned long maxOutputBytes;
    //输出数据
    unsigned char *outputBuffer = 0;
    //输入容器大小
    unsigned long inputByteNum;
};


#endif //LIVEDEMO_AUDIOCHANNEL_H
