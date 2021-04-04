package cn.leizy.live

import androidx.camera.core.CameraSelector
import androidx.camera.view.PreviewView
import androidx.core.app.ComponentActivity
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleOwner

/**
 * @author wulei
 * @date 4/3/21
 * @description
 */
class LivePusher(
    private val context: ComponentActivity,
    private val width: Int = 0,
    private val height: Int = 0,
    private val bitrate: Int = 800_000,
    private val fps: Int = 10,
    private val cameraId: Int = CameraSelector.LENS_FACING_BACK,
    private val previewView: PreviewView,
    //音频参数
    private val sampleRate: Int = 44100,
    private val channels: Int = 2
) {
    private var url: String? = null
    private var isStart: Boolean = false
    private var videoChannel: VideoChannel
    private var audioChannel: AudioChannel

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    init {
        native_init()
        videoChannel =
            VideoChannel(context, previewView, this, width, height, bitrate, fps, cameraId)
        audioChannel = AudioChannel(sampleRate, channels, this)
    }

    fun startLive(url: String) {
        if (!isStart) {
            this.url = url
            native_start(url)
            videoChannel.startLive()
            audioChannel.startLive()
            isStart = true
        }
    }

    fun resumeLive() {
        if (isStart) {
            url?.apply {
                native_start(this)
                videoChannel.startLive()
                audioChannel.startLive()
            }
        }
    }

    fun pauseLive() {
        if (isStart) {
            native_stop()
            videoChannel.stopLive()
            audioChannel.stopLive()
        }
    }

    fun stopLive() {
        if (isStart) {
            native_stop()
            videoChannel.stopLive()
            audioChannel.stopLive()
            isStart = false
        }
    }

    fun switchCamera() {
        videoChannel.switchCamera()
    }

    external fun native_init()

    external fun native_setVideoEncodeInfo(width: Int, height: Int, fps: Int, bitrate: Int)

    external fun native_start(url: String)

    external fun native_setAudioEncodeInfo(sampleRate: Int, channels: Int): Int

    external fun native_pushVideo(data: ByteArray)

    external fun native_stop()

    external fun native_release()

    external fun native_pushAudio(buffer: ByteArray, len: Int)
}