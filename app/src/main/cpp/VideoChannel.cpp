//
// Created by 吴磊 on 4/4/21.
//

#include <cstring>
#include "VideoChannel.h"
#include "mylog.h"

VideoChannel::VideoChannel() {

}

VideoChannel::~VideoChannel() {
    if (videoCodec) {
        LOGW("release VideoChannel");
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }
}

void VideoChannel::setVideoEncodeInfo(int width, int height, int fps, int bitrate) {
    LOGW("%d,%d,%d,%d", width, height, fps, bitrate);
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
    memcpy(pic_in->img.plane[0], data, ySize);
    for (int i = 0; i < uvSize; ++i) {
        *(pic_in->img.plane[2] + i) = *(data + ySize + i * 2 + 1);//v数据
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2);//u数据
    }
    //264
    int pi_nal;
    //编码出的数据
    x264_nal_t *pp_nals;
    //编码出的参数 BufferInfo
    x264_picture_t pic_out;
    //yuv数据转化x264
    x264_encoder_encode(videoCodec, &pp_nals, &pi_nal, pic_in, &pic_out);
//    LOGW("videoCodec value %d ", videoCodec);

    uint8_t sps[100];
    uint8_t pps[100];

    int sps_len, pps_len;
//    LOGW("编码出的帧数 %d", pi_nal);
    if (pi_nal > 0) {
        for (int i = 0; i < pi_nal; ++i) {
//            LOGE("i : %d; length : %d", i, pi_nal);
            if (pp_nals[i].i_type == NAL_SPS) {
                sps_len = pp_nals[i].i_payload - 4;
                memcpy(sps, pp_nals[i].p_payload + 4, sps_len);
            } else if (pp_nals[i].i_type == NAL_PPS) {
                pps_len = pp_nals[i].i_payload - 4;
                memcpy(pps, pp_nals[i].p_payload + 4, pps_len);
                sendSpsPps(sps, pps, sps_len, pps_len);
            } else {
                sendFrame(pp_nals[i].i_type, pp_nals[i].i_payload, pp_nals[i].p_payload);
            }
        }
    }
//    LOGE("pi_nal %d", pi_nal);
}

void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    RTMPPacket *packet = new RTMPPacket;
    int bodySize = 13 + sps_len + 3 + pps_len;
    RTMPPacket_Alloc(packet, bodySize);
    int i = 0;
    //AVC sequence header 与IDR一样
    packet->m_body[i++] = 0x17;
    //AVC sequence header 设置为0x00
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    //AVC sequence header
    packet->m_body[i++] = 0x01;//版本号1
    packet->m_body[i++] = sps[1]; // profile
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];// profile level
    packet->m_body[i++] = 0xFF;

    //sps
    packet->m_body[i++] = 0xE1;
    packet->m_body[i++] = (sps_len >> 8) & 0xff;
    packet->m_body[i++] = sps_len & 0xff;

    memcpy(&packet->m_body[i], sps, sps_len);
    i += sps_len;

    //pps
    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = pps_len & 0xff;
    memcpy(&packet->m_body[i], pps, pps_len);

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodySize;
    packet->m_nChannel = 10;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    if (this->callback) {
        this->callback(packet);
    }
}

void VideoChannel::sendFrame(int type, int payload, uint8_t *p_payload) {
    //去掉分隔符
    if (p_payload[2] == 0x00) {
        payload -= 4;
        p_payload += 4;
    } else if (p_payload[2] == 0x01) {
        payload -= 3;
        p_payload += 3;
    }
    RTMPPacket *packet = new RTMPPacket;
    int bodySize = 9 + payload;
    RTMPPacket_Alloc(packet, bodySize);
    RTMPPacket_Reset(packet);

    packet->m_body[0] = 0x27;
    if (type == NAL_SLICE_IDR) {
        LOGD("send key frame");
        packet->m_body[0] = 0x17;
    }
    //类型
    packet->m_body[1] = 0x01;
    //时间戳
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;
    //长度
    packet->m_body[5] = (payload >> 24) & 0xff;
    packet->m_body[6] = (payload >> 16) & 0xff;
    packet->m_body[7] = (payload >> 8) & 0xff;
    packet->m_body[8] = payload & 0xff;

    memcpy(&packet->m_body[9], p_payload, payload);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    if (this->callback) {
        this->callback(packet);
    }
}

void VideoChannel::setVideoCallback(VideoChannel::VideoCallback callback) {
    this->callback = callback;
}
