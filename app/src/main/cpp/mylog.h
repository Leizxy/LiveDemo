//
// Created by 吴磊 on 4/4/21.
//

#ifndef LIVEDEMO_MYLOG_H
#define LIVEDEMO_MYLOG_H

#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"live",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"live",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"live",__VA_ARGS__)
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE,"live",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,"live",__VA_ARGS__)

#endif //LIVEDEMO_MYLOG_H
