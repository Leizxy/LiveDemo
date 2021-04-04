//
// Created by 吴磊 on 4/4/21.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel() {

}

VideoChannel::~VideoChannel() {

}

void VideoChannel::setVideoEncodeInfo(int width, int height, int fps, int bitrate) {
    mWidth = width;
    mHeight = height;
    mFps = fps;
    mBitrate = bitrate;

    ySize = width * height;
    uvSize = ySize / 4;

    if (videoCodec) {
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }
    //定义参数
    x264_param_t param;
    //参数赋值 ultrafast zerolatency
    x264_param_default_preset(&param, x264_preset_names[0], x264_tune_names[7]);
    //编码等级
    param.i_level_idc = 32;
    //选取显示格式
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    //B帧个数
    param.i_bframe = 0;
    //ABR平均
    param.rc.i_rc_method = X264_RC_ABR;
    param.rc.i_bitrate = bitrate / 1024;
    //帧率
    param.i_fps_num = fps;
    param.i_fps_den = 1;
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;

    param.b_vfr_input = 0;
    //I帧间隔
    param.i_keyint_max = fps * 2;
    //是否复制sps和pps放在每个关键帧的前面，每个I帧都附带sps/pps
    param.b_repeat_headers = 1;
    //多线程
    param.i_threads = 1;
    // baseline
    x264_param_apply_profile(&param, x264_profile_names[0]);
    //打开编码器 宽高一定是交换的
    videoCodec = x264_encoder_open(&param);
    //容器
    pic_in = new x264_picture_t;
    //设置初始化大小
    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);
}

void VideoChannel::encodeData(int8_t *data) {

}
