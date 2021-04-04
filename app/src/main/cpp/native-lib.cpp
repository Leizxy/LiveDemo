#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_cn_leizy_live_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1init(JNIEnv *env, jobject thiz) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1setVideoEncodeInfo(JNIEnv *env, jobject thiz, jint width,
                                                         jint height, jint fps, jint bitrate) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1start(JNIEnv *env, jobject thiz, jstring path) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1setAudioEncodeInfo(JNIEnv *env, jobject thiz,
                                                         jint sample_rate, jint channels) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz, jbyteArray data) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1stop(JNIEnv *env, jobject thiz) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1release(JNIEnv *env, jobject thiz) {

}extern "C"
JNIEXPORT void JNICALL
Java_cn_leizy_live_LivePusher_native_1pushAudio(JNIEnv *env, jobject thiz, jbyteArray buffer,
                                                jint len) {

}