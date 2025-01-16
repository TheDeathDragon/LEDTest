#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#define PIN D2                 // NeoPixel 数据引脚
#define NUMPIXELS 10           // LED 数量
#define BRIGHTNESS 50          // LED 全局亮度
#define DELAY_SHORT 50         // 辅助短延时 (ms)
#define DELAY_MED 200          // 辅助中等延时 (ms)
#define DELAY_LONG 1000        // 辅助长延时 (ms)

const char *ssid = "ESP_8266";
const char *password = "12345678";

ESP8266WebServer server(80);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void clearPixels() {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, 0);
  }
  pixels.show();
}

void setAllPixels(uint32_t color) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

// 呼吸灯效果：color 颜色, durationMs 单次渐变总时长, loops 循环次数
// 注意：此处 color 参数若(0,255,0)则实际显示为红色；(255,0,0)则显示为绿色
void breathingEffect(uint32_t color, int durationMs, int loops) {
  // 简单线性渐变示例
  for (int loop = 0; loop < loops; loop++) {
    // 从暗到亮
    for (int val = 0; val <= 255; val += 5) {
      float brightness = val / 255.0;
      for (int j = 0; j < NUMPIXELS; j++) {
        pixels.setPixelColor(
          j,
          pixels.Color(
            (uint8_t)(brightness * ((color >> 16) & 0xFF)),
            (uint8_t)(brightness * ((color >> 8) & 0xFF)),
            (uint8_t)(brightness * (color & 0xFF))
          )
        );
      }
      pixels.show();
      // 255/5 = 51 步，durationMs / 2 / 51 控制每次亮度改变的间隔
      delay(durationMs / 2 / (255 / 5));
    }
    // 从亮到暗
    for (int val = 255; val >= 0; val -= 5) {
      float brightness = val / 255.0;
      for (int j = 0; j < NUMPIXELS; j++) {
        pixels.setPixelColor(
          j,
          pixels.Color(
            (uint8_t)(brightness * ((color >> 16) & 0xFF)),
            (uint8_t)(brightness * ((color >> 8) & 0xFF)),
            (uint8_t)(brightness * (color & 0xFF))
          )
        );
      }
      pixels.show();
      delay(durationMs / 2 / (255 / 5));
    }
  }
}

/**************************************************************
 * 各种灯效函数（阻塞式）
 **************************************************************/

// 1.总开关 - 关闭所有灯效
void handleTotalOff() {
  clearPixels();
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>所有灯效已关闭</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 2.来电灯效：红和蓝交替闪烁 (均 200ms)，持续 5 秒
void handleEffectCallIn() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    // 红
    setAllPixels(pixels.Color(0,255,0));
    delay(200);
    clearPixels();
    delay(200);

    // 蓝
    setAllPixels(pixels.Color(0,0,255));
    delay(200);
    clearPixels();
    delay(200);
  }
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>来电灯效执行完毕</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 3.通知灯效：绿色呼吸，循环 5 次
void handleEffectNotification() {
  // 绿
  breathingEffect(pixels.Color(255,0,0), 2000, 5);
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>通知灯效执行完毕</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 4.音乐灯效：蓝色以 200ms 闪烁，示例闪烁 5 秒
void handleEffectMusic() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    // 蓝
    setAllPixels(pixels.Color(0,0,255));
    delay(200);
    clearPixels();
    delay(200);
  }
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>音乐灯效演示结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 5.蓝牙连接中灯效：蓝色快速闪烁 (300ms 亮, 500ms 灭)，5 秒
void handleEffectBTConnecting() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    setAllPixels(pixels.Color(0,0,255)); // 蓝
    delay(300);
    clearPixels();
    delay(500);
  }
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>蓝牙连接中演示结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 6.蓝牙连接成功灯效：蓝色常亮 3 秒
void handleEffectBTSuccess() {
  setAllPixels(pixels.Color(0,0,255)); // 蓝
  delay(3000);
  clearPixels();
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>蓝牙连接成功演示结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 7.蓝牙连接失败灯效：红色快速闪烁 (200ms 亮, 400ms 灭)，5 秒
void handleEffectBTFail() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    setAllPixels(pixels.Color(0,255,0)); // 红
    delay(200);
    clearPixels();
    delay(400);
  }
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>蓝牙连接失败演示结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 8.WIFI连接中灯效：绿色呼吸闪烁 (1 秒渐变)，持续 5 秒
void handleEffectWiFiConnecting() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    // 绿
    breathingEffect(pixels.Color(255,0,0), 1000, 1);
  }
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>WiFi连接中演示结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 9.WIFI连接成功灯效：绿色常亮 3 秒
void handleEffectWiFiSuccess() {
  setAllPixels(pixels.Color(255,0,0)); // 绿
  delay(3000);
  clearPixels();
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>WiFi连接成功演示结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 10.WIFI连接失败灯效：红色快速闪烁 (300ms 亮, 300ms 灭)，5 秒
void handleEffectWiFiFail() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    setAllPixels(pixels.Color(0,255,0)); // 红
    delay(300);
    clearPixels();
    delay(300);
  }
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>WiFi连接失败演示结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 11.Party灯效：依次闪烁红、绿、蓝，100ms 闪烁，持续 10 秒
void handleEffectParty() {
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {
    // 红
    setAllPixels(pixels.Color(0,255,0));
    delay(100);
    clearPixels();
    delay(100);

    // 绿
    setAllPixels(pixels.Color(255,0,0));
    delay(100);
    clearPixels();
    delay(100);

    // 蓝
    setAllPixels(pixels.Color(0,0,255));
    delay(100);
    clearPixels();
    delay(100);
  }
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>Party灯效结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 12.充电低电量灯效：红色呼吸，循环 5 次
void handleEffectChargeLow() {
  breathingEffect(pixels.Color(0,255,0), 2000, 5); // 红
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>充电低电量灯效结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 13.充电高电量灯效：绿色呼吸，循环 5 次
void handleEffectChargeHigh() {
  breathingEffect(pixels.Color(255,0,0), 2000, 5); // 绿
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>充电高电量灯效结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 14.充电完成灯效：蓝色常亮 3 秒
void handleEffectChargeDone() {
  setAllPixels(pixels.Color(0,0,255)); // 蓝
  delay(3000);
  clearPixels();
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>充电完成灯效（演示）结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 15.相机对焦灯效：红色亮起并持续 2 秒
void handleEffectCamFocus() {
  setAllPixels(pixels.Color(0,255,0)); // 红
  delay(2000);
  clearPixels();
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>相机对焦灯效结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 16.相机拍摄灯效：蓝色闪烁，持续 1 秒
void handleEffectCamShoot() {
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {
    setAllPixels(pixels.Color(0,0,255)); // 蓝
    delay(100);
    clearPixels();
    delay(100);
  }
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>相机拍摄灯效结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 17.相机保存照片灯效：绿色亮起 2 秒
void handleEffectCamSave() {
  setAllPixels(pixels.Color(255,0,0)); // 绿
  delay(2000);
  clearPixels();
  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>相机保存照片灯效结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

// 18.开机灯效：红 -> 绿 -> 蓝，依次点亮并保持 1 秒，间隔 0.2 秒
void handleEffectStartup() {
  // 红 => (0,255,0)
  setAllPixels(pixels.Color(0,255,0));
  delay(1000);
  clearPixels();
  delay(200);

  // 绿 => (255,0,0)
  setAllPixels(pixels.Color(255,0,0));
  delay(1000);
  clearPixels();
  delay(200);

  // 蓝 => (0,0,255)
  setAllPixels(pixels.Color(0,0,255));
  delay(1000);
  clearPixels();

  server.send(200, "text/html",
              "<html>"
              "<head><meta charset='UTF-8'></head>"
              "<body><h1>开机灯效结束</h1><a href='/'>返回</a></body>"
              "</html>");
}

/**************************************************************
 * HTTP处理
 **************************************************************/
void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'></head><body>";
  html += "<h1>LED 灯效控制</h1>";
  html += "<p><a href='/totaloff'>关闭所有灯效</a></p>";
  html += "<p><a href='/callin'>来电</a> | <a href='/notification'>通知</a> | <a href='/music'>音乐</a></p>";
  html += "<p><a href='/bt_connecting'>蓝牙连接中</a> | <a href='/bt_success'>蓝牙成功</a> | <a href='/bt_fail'>蓝牙失败</a></p>";
  html += "<p><a href='/wifi_connecting'>WiFi连接中</a> | <a href='/wifi_success'>WiFi成功</a> | <a href='/wifi_fail'>WiFi失败</a></p>";
  html += "<p><a href='/party'>Party</a> | <a href='/charge_low'>充电低</a> | <a href='/charge_high'>充电高</a> | <a href='/charge_done'>充电完成</a></p>";
  html += "<p><a href='/cam_focus'>相机对焦</a> | <a href='/cam_shoot'>相机拍摄</a> | <a href='/cam_save'>相机保存</a></p>";
  html += "<p><a href='/startup'>开机</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);
  clearPixels();

  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP已启动，IP地址:");
  Serial.println(WiFi.softAPIP());

  // 设置 HTTP 服务器路由
  server.on("/", handleRoot);
  server.on("/totaloff", handleTotalOff);
  server.on("/callin", handleEffectCallIn);
  server.on("/notification", handleEffectNotification);
  server.on("/music", handleEffectMusic);
  server.on("/bt_connecting", handleEffectBTConnecting);
  server.on("/bt_success", handleEffectBTSuccess);
  server.on("/bt_fail", handleEffectBTFail);
  server.on("/wifi_connecting", handleEffectWiFiConnecting);
  server.on("/wifi_success", handleEffectWiFiSuccess);
  server.on("/wifi_fail", handleEffectWiFiFail);
  server.on("/party", handleEffectParty);
  server.on("/charge_low", handleEffectChargeLow);
  server.on("/charge_high", handleEffectChargeHigh);
  server.on("/charge_done", handleEffectChargeDone);
  server.on("/cam_focus", handleEffectCamFocus);
  server.on("/cam_shoot", handleEffectCamShoot);
  server.on("/cam_save", handleEffectCamSave);
  server.on("/startup", handleEffectStartup);

  server.begin();
  Serial.println("HTTP服务器已启动");
}

void loop() {
  server.handleClient();
}