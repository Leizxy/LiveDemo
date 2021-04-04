#include <jni.h>
#include <string>
#include "VideoChannel.h"
#include "mylog.h"
#include "safe_queue.h"
#include <pthread.h>

VideoChannel *videoChannel = 0;
int isStart = 0;
//记录子线程对象
pthread_t pid = 0;
//推流 标志
int readyPushing = 0;
//阻塞队列 用于推流数据
SafeQueue<RTMPPacket *> packets;

uint32_t start_time;
RTMP *rtmp = 0;
//虚拟机引用
JavaVM *javaVm = 0;


void releasePackets(RTMPPacket *packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }
}

void videoCallback(RTMPPacket *packet) {
    if (packet) {
        if (packets.size() > 50) {
            packets.clear();
        }
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        packets.push(packet);
    }
}

void *start(void *args) {
    char *url = static_cast<char *>(args);
    LOGW("start connect %s", url);
    do {
        rtmp = RTMP_Alloc();
        if (!rtmp) {
            LOGE("create rtmp failed");
            break;
        }
        RTMP_Init(rtmp);
        rtmp->Link.timeout = 5;//超时时间
        int ret = RTMP_SetupURL(rtmp, url);
        if (!ret) {
            LOGE("RTMP set url fail -> %s", url);
            break;
        }
        //开启输出模式
        RTMP_EnableWrite(rtmp);
        ret = RTMP_Connect(rtmp, 0);
        if (!ret) {
            LOGE("RTMP connect fail -> %s", url);
            break;
        }
        ret = RTMP_ConnectStream(rtmp, 0);
        LOGI("RTMP connect success");
        if (!ret) {
            LOGE("RTMP connect stream fail -> %s", url);
            break;
        }
        readyPushing = 1;
        start_time = RTMP_GetTime();//记录开始推流时间
        packets.setWork(1);
        RTMPPacket *packet = 0;

        while (isStart) {
            packets.pop(packet);
            if (!isStart) {
                break;
            }
            if (!packet) {
                continue;
            }
            packet->m_nInfoField2 = rtmp->m_stream_id;
            ret = RTMP_SendPacket(rtmp, packet, 1);//发送包
            releasePackets(packet);
            if (!ret) {
                LOGE("send packet fail");
                break;
            }
        }
        releasePackets(packet);
    } while (0);
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete url;
    return 0; //不return会报错。
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVm = vm;
    LOGW("JNI_OnLoad %s", "load vm");
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1init(JNIEnv *env, jobject thiz) {
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallback(videoCallback);
}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1setVideoEncodeInfo(JNIEnv *env, jobject thiz, jint width,
                                                         jint height, jint fps, jint bitrate) {
    if (videoChannel) {
        videoChannel->setVideoEncodeInfo(width, height, fps, bitrate);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1start(JNIEnv *env, jobject thiz, jstring path_) {
    LOGI("is Start %d", isStart);
    if (isStart) return;
    const char *path = env->GetStringUTFChars(path_, 0);
    char *url = new char[strlen(path) + 1];
    strcpy(url, path);
    isStart = 1;
    pthread_create(&pid, 0, start, url);
    env->ReleaseStringUTFChars(path_, path);
}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1setAudioEncodeInfo(JNIEnv *env, jobject thiz,
                                                         jint sample_rate, jint channels) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz, jbyteArray data_) {
    if (!videoChannel || !readyPushing) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    videoChannel->encodeData(data);
    env->ReleaseByteArrayElements(data_, data, 0);
}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1stop(JNIEnv *env, jobject thiz) {
    isStart = 0;
}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1release(JNIEnv *env, jobject thiz) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1pushAudio(JNIEnv *env, jobject thiz, jbyteArray buffer,
                                                jint len) {

}