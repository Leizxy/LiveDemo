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

    private val url: String =
        "rtmp://live-push.bilivideo.com/live-bvc/?streamname=live_8379703_1896767&key=167ddc29b2d089fe2e43f58fe3b8c36d&schedule=rtmp&pflag=1"

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
                    Manifest.permission.CAMERA,
                    Manifest.permission.RECORD_AUDIO
                ), 1
            )
        }
    }

    override fun onResume() {
        super.onResume()
        livePusher.resumeLive()
    }

    override fun onStop() {
        super.onStop()
        livePusher.pauseLive()
    }

    override fun onDestroy() {
        super.onDestroy()
        livePusher.native_release()
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