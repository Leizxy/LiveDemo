package cn.leizy.live

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import cn.leizy.live.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {
    private lateinit var livePusher: LivePusher
    private lateinit var binding: ActivityMainBinding

    private val url: String = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding =
            DataBindingUtil.setContentView(this, R.layout.activity_main)
        permissions()
        livePusher = LivePusher(this, previewView = binding.preview)
    }

    private fun permissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(
                arrayOf(
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.CAMERA
                ), 1
            )
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }

    fun startLive(view: View) {
        livePusher.startLive(url)
    }

    fun switchCamera(view: View) {
        livePusher.switchCamera()
    }

    fun stopLive(view: View) {
        livePusher.stopLive()
    }
}