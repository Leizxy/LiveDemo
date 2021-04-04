package cn.leizy.live

import android.util.DisplayMetrics
import android.util.Log
import android.util.Size
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.app.ComponentActivity
import androidx.core.content.ContextCompat
import java.lang.Exception
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import java.util.concurrent.locks.ReentrantLock
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min

/**
 * @author wulei
 * @date 4/3/21
 * @description
 */
class VideoChannel(
    private val context: ComponentActivity,
    private val previewView: PreviewView,
    private val pusher: LivePusher,
    private var width: Int,
    private var height: Int,
    private var bitrate: Int,
    private var fps: Int,
    private var lensFacing: Int
) : ImageAnalysis.Analyzer {
    companion object {
        private const val RATIO_4_3_VALUE = 4.0 / 3.0
        private const val RATIO_16_9_VALUE = 16.0 / 9.0
    }

    private var cameraExecutor: ExecutorService = Executors.newSingleThreadExecutor()

    //    private var lensFacing: Int = CameraSelector.LENS_FACING_BACK
    private var cameraProvider: ProcessCameraProvider? = null
    private var isLiving: Boolean = false
    private val lock: ReentrantLock = ReentrantLock()
    private lateinit var y: ByteArray
    private lateinit var u: ByteArray
    private lateinit var v: ByteArray
    private lateinit var nv21: ByteArray
    private lateinit var nv21_rotated: ByteArray

    init {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(context)
        cameraProviderFuture.addListener({
            cameraProvider = cameraProviderFuture.get()
            lensFacing = when {
                hasBackCamera() -> CameraSelector.LENS_FACING_BACK
                hasFrontCamera() -> CameraSelector.LENS_FACING_FRONT
                else -> throw IllegalStateException("Back and front camera are unavailable")
            }
            bindCameraUseCases()
        }, ContextCompat.getMainExecutor(context))
    }

    fun startLive() {
        isLiving = true
    }

    fun stopLive() {
        isLiving = false
    }

    private fun bindCameraUseCases() {
        val metrics = DisplayMetrics().also { previewView.display.getRealMetrics(it) }
        Log.i(
            "VideoChannel",
            "bindCameraUseCases: ${metrics.widthPixels} X ${metrics.heightPixels}"
        )
        val screenAspectRatio =
            if ((width == 0) or (height == 0))
                aspectRatio(width, height)
            else
                aspectRatio(metrics.widthPixels, metrics.heightPixels)
        Log.i("VideoChannel", "bindCameraUseCases: $screenAspectRatio")

        val rotation = previewView.display.rotation

        val cameraProvider =
            cameraProvider ?: throw IllegalStateException("Camera initialization failed.")
        val cameraSelector = CameraSelector.Builder().requireLensFacing(lensFacing).build()

        val preview = Preview.Builder()
            .setTargetAspectRatio(screenAspectRatio)
            .setTargetRotation(rotation)
            .build()

        val imgAnalyzer = ImageAnalysis.Builder().setTargetAspectRatio(screenAspectRatio)
            .setTargetRotation(rotation)
            .build().also {
                it.setAnalyzer(cameraExecutor, this)
            }

        cameraProvider.unbindAll()

        try {
            cameraProvider.bindToLifecycle(context, cameraSelector, preview, imgAnalyzer)
            preview.setSurfaceProvider(previewView.surfaceProvider)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun aspectRatio(width: Int, height: Int): Int {
        val previewRatio = max(width, height).toDouble() / min(width, height)
        if (abs(previewRatio - RATIO_4_3_VALUE) <= abs(previewRatio - RATIO_16_9_VALUE)) {
            return AspectRatio.RATIO_4_3
        }
        return AspectRatio.RATIO_16_9
    }

    private fun hasFrontCamera(): Boolean {
        return cameraProvider?.hasCamera(CameraSelector.DEFAULT_FRONT_CAMERA) ?: false
    }

    private fun hasBackCamera(): Boolean {
        return cameraProvider?.hasCamera(CameraSelector.DEFAULT_BACK_CAMERA) ?: false
    }

    override fun analyze(image: ImageProxy) {
//        Log.i("VideoChannel", "analyze: ${image.width} X ${image.height}")
        if (!isLiving) {
            image.close()
            return
        }
        lock.lock()
        val planes = image.planes
        if (!this::y.isInitialized) {
            y = ByteArray(planes[0].buffer.limit() - planes[0].buffer.position())
            u = ByteArray(planes[1].buffer.limit() - planes[1].buffer.position())
            v = ByteArray(planes[2].buffer.limit() - planes[2].buffer.position())
            pusher.native_setVideoEncodeInfo(image.height, image.width, fps, bitrate)//todo rotate控制
        }
        if (image.planes[0].buffer.remaining() == y.size) {
            planes[0].buffer.get(y)
            planes[1].buffer.get(u)
            planes[2].buffer.get(v)
            val stride = planes[0].rowStride
            val size = Size(image.width, image.height)
            val width = size.height
            val height = size.width
            if (!this::nv21.isInitialized) {
                nv21 = ByteArray(height * width * 3 / 2)
                nv21_rotated = ByteArray(height * width * 3 / 2)
            }
            ImageUtil.yuvToNv21(y, u, v, nv21, height, width)
            JImageUtil.nv21_rotate_to_90(nv21, nv21_rotated, height, width)//todo rotate
            pusher.native_pushVideo(nv21_rotated)
        }
        lock.unlock()
        image.close()
    }

    fun switchCamera() {

    }
}