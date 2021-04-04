//
// Created by 吴磊 on 4/4/21.
//

#include <cstring>
#include <malloc.h>
#include "AudioChannel.h"
#include "mylog.h"

AudioChannel::AudioChannel() {

}

AudioChannel::~AudioChannel() {

}

void AudioChannel::openCodec(int sampleRate, int channels) {
    LOGI("openCodec");
    unsigned long inputSamples;
    codec = faacEncOpen(sampleRate, channels, &inputSamples, &maxOutputBytes);
    //输入容器大小
    inputByteNum = inputSamples * 2;
    //实例化输出容器
    outputBuffer = static_cast<unsigned char *>(malloc(maxOutputBytes));
    LOGI("codec:%d; inputByteNum:%d; maxOutputBytes:%d", codec, inputByteNum, maxOutputBytes);
    //参数
    faacEncConfigurationPtr configurationPtr = faacEncGetCurrentConfiguration(codec);
    //编码 MPEG AAC
    configurationPtr->mpegVersion = MPEG4;
    //编码等级
    configurationPtr->aacObjectType = LOW;
    //输出aac裸流数据
    configurationPtr->outputFormat = 0;
    //采样位数
    configurationPtr->inputFormat = FAAC_INPUT_16BIT;
    //配置生效
    faacEncSetConfiguration(codec, configurationPtr);
    LOGI("config set");
}

void AudioChannel::encode(int32_t *data, int len) {
    LOGE("push audio %d", len);
    int byteLen = faacEncEncode(codec, data, len, outputBuffer, maxOutputBytes);
    LOGE("push %d", byteLen);
    if (byteLen > 0) {
        //拼装RTMP数据包
        RTMPPacket *packet = new RTMPPacket;
        RTMPPacket_Alloc(packet, byteLen + 2);
        //固定数据
        packet->m_body[0] = 0xAF;
        packet->m_body[1] = 0x01;
        memcpy(&packet->m_body[2], outputBuffer, byteLen);
        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = byteLen + 2;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 12;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        if (this->callback) {
            LOGE("callback audio %d", len);
            callback(packet);
        }
    }
}

RTMPPacket *AudioChannel::getAudioConfig() {
    LOGI("config");
    u_char *buf;
    u_long len;
    //头帧内容
    faacEncGetDecoderSpecificInfo(codec, &buf, &len);
    //头帧的rtmpdump 实时录制 实时给时间戳
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, len + 2);

    packet->m_body[0] = 0xAF;
    packet->m_body[1] = 0x01;
    memcpy(&packet->m_body[2], buf, len);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = len + 2;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 12;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    return packet;
}
