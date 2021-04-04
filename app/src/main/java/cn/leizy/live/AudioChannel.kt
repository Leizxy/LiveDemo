package cn.leizy.live

import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import java.util.concurrent.*

/**
 * @author wulei
 * @date 4/4/21
 * @description
 */
class AudioChannel(
    private val sampleRate: Int,
    channels: Int,
    private val pusher: LivePusher
) {
    private var isPushing: Boolean = false
    private var handler: Handler
    private var handlerThread: HandlerThread
    private val channelConfig: Int
    private var minBufferSize: Int
    private var buffer: ByteArray
    private lateinit var audioRecord: AudioRecord
    private val executor = Executors.newSingleThreadExecutor() /*{
        val thread = Thread()
        thread.priority = Thread.NORM_PRIORITY
        thread.name = "audio_thread"
        thread
    }*/

    init {
        Executors.newSingleThreadExecutor()
        channelConfig =
            if (channels == 2) AudioFormat.CHANNEL_IN_STEREO else AudioFormat.CHANNEL_IN_MONO
        minBufferSize =
            AudioRecord.getMinBufferSize(sampleRate, channelConfig, AudioFormat.ENCODING_PCM_16BIT)
        //初始化faac
        val inputByteNum = pusher.native_setAudioEncodeInfo(sampleRate, channels)
        buffer = ByteArray(inputByteNum)
        //
        minBufferSize = if (inputByteNum > minBufferSize) inputByteNum else minBufferSize

        handlerThread = HandlerThread("audio_handler");
        handlerThread.start()
        handler = Handler(handlerThread.looper)
    }

    fun startLive() {
        Log.i("AudioChannel", "startLive: q")
        val thread = Thread {
            Log.i("AudioChannel", "startLive: ")
            isPushing = true
            if (!this::audioRecord.isInitialized) {
                audioRecord = AudioRecord(
                    MediaRecorder.AudioSource.MIC,
                    sampleRate,
                    channelConfig,
                    AudioFormat.ENCODING_PCM_16BIT,
                    minBufferSize
                )
            }
            audioRecord.startRecording()
            while (isPushing && audioRecord.recordingState == AudioRecord.RECORDSTATE_RECORDING) {
                val len = audioRecord.read(buffer, 0, buffer.size)
                Log.i("AudioChannel", "startLive: $len")
                if (len > 0) {
                    pusher.native_pushAudio(buffer, len / 2)
                }
            }
        }
//        thread.start()
        executor.execute(thread)

//        handler.post {
//            Log.i("AudioChannel", "startLive: ")
//            audioRecord = AudioRecord(
//                MediaRecorder.AudioSource.MIC,
//                sampleRate,
//                channelConfig,
//                AudioFormat.ENCODING_PCM_16BIT,
//                minBufferSize
//            )
//            audioRecord.startRecording()
//            while (audioRecord.recordingState == AudioRecord.RECORDSTATE_RECORDING) {
//                val len = audioRecord.read(buffer, 0, buffer.size)
//                Log.i("AudioChannel", "startLive: $len")
//                if (len > 0) {
//                    pusher.native_pushAudio(buffer, len / 2)
//                }
//            }
//        }
    }

    fun stopLive() {
        isPushing = false
    }
}