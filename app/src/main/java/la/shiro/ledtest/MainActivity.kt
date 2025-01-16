package la.shiro.ledtest

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.GridItemSpan
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import okhttp3.OkHttpClient
import okhttp3.Request
import java.util.concurrent.TimeUnit

val paths = listOf(
    "totaloff",
    "callin",
    "notification",
    "music",
    "bt_connecting",
    "bt_success",
    "bt_fail",
    "wifi_connecting",
    "wifi_success",
    "wifi_fail",
    "party",
    "charge_low",
    "charge_high",
    "charge_done",
    "cam_focus",
    "cam_shoot",
    "cam_save",
    "startup"
)
val pathName = listOf(
    "关闭所有灯效",
    "来电灯效", "通知灯效", "音乐灯效",
    "蓝牙连接中", "蓝牙成功", "蓝牙失败",
    "WiFi连接中", "WiFi成功", "WiFi失败",
    "Party灯效", "充电低", "充电高", "充电完成",
    "相机对焦", "相机拍摄", "相机保存",
    "开机灯效"
)

class MainActivity : ComponentActivity() {

    private val client by lazy { OkHttpClient.Builder().readTimeout(60, TimeUnit.SECONDS).build() }
    private val baseUrl = "http://192.168.4.1"
    private var isLastEffectFinished = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            ESP8266GridDemoScreen(
                onSendRequest = { path, onResult ->
                    sendHttpRequest(path, onResult)
                }
            )
        }
    }

    private fun sendHttpRequest(path: String, onResult: (Boolean, String) -> Unit) {

        if (!isLastEffectFinished) {
            onResult(false, "上一个灯效演示还未结束")
            return
        }
        CoroutineScope(Dispatchers.IO).launch {
            val request = Request.Builder()
                .url("$baseUrl/$path")
                .build()

            try {
                isLastEffectFinished = false
                client.newCall(request).execute()
                withContext(Dispatchers.Main) {
                    onResult(true, "灯效 [${pathName[paths.indexOf(path)]}] 演示结束")
                    isLastEffectFinished = true
                }
            } catch (e: Exception) {
                e.printStackTrace()
                withContext(Dispatchers.Main) {
                    onResult(false, "请求失败: ${e.message}, 当前灯效 [${pathName[paths.indexOf(path)]}]")
                    isLastEffectFinished = true
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ESP8266GridDemoScreen(
    onSendRequest: (String, (Boolean, String) -> Unit) -> Unit
) {
    var statusText by remember { mutableStateOf("等待操作...") }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("灯效示例") }
            )
        }
    ) { innerPadding ->
        LazyVerticalGrid(
            columns = GridCells.Fixed(2),
            modifier = Modifier
                .fillMaxSize()
                .padding(innerPadding),
            horizontalArrangement = Arrangement.Center,
            verticalArrangement = Arrangement.Center
        ) {
            item(span = { GridItemSpan(maxLineSpan) }) {
                Text(
                    text = statusText,
                    style = MaterialTheme.typography.bodyMedium,
                    textAlign = TextAlign.Center,
                    modifier = Modifier.padding(16.dp)
                )
            }
            items(paths) { path ->
                Button(
                    onClick = {
                        statusText = "正在演示 [${pathName[paths.indexOf(path)]}] 灯效..."
                        onSendRequest(path) { success, message ->
                            statusText = if (success) {
                                message
                            } else {
                                message
                            }
                        }
                    },
                    modifier = Modifier
                        .padding(8.dp)
                        .fillMaxWidth()
                ) {
                    Text(
                        text = pathName[paths.indexOf(path)],
                        textAlign = TextAlign.Center,
                        maxLines = 1
                    )
                }
            }
        }
    }
}