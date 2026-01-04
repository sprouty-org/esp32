#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h> 
#include "esp_camera.h"
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <DHT.h>
#include <time.h> 

// --- Configuration ---
#define SERVER_IP "10.117.113.22"
#define SERVER_PORT 8080
#define SERVER_URL_DATA "http://10.117.113.22:8080/sensors/data"
#define SERVER_PATH_IMAGE "/sensors/image"

// --- NTP Settings ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

// --- Timing ---
#define uS_TO_HOUR 3600000000ULL
#define DATA_INTERVAL_HOURS 6

// --- RTC ---
RTC_DATA_ATTR uint8_t rtc_bssid[6];
RTC_DATA_ATTR uint8_t rtc_channel = 0;
RTC_DATA_ATTR bool rtc_valid = false;

// --- Pins ---
#define BOOT_BUTTON_PIN 0  
#define SOIL_PIN 1 
#define DHT_PIN 2 
#define DHT_TYPE DHT11
#define OLED_SDA 42
#define OLED_SCL 41

// --- Camera Pins ---
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5
#define Y9_GPIO_NUM 16
#define Y8_GPIO_NUM 17
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 12
#define Y5_GPIO_NUM 10
#define Y4_GPIO_NUM 8
#define Y3_GPIO_NUM 9
#define Y2_GPIO_NUM 11
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13

String deviceID = "";
Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(DHT_PIN, DHT_TYPE);

const char* SPROUTY_CSS = R"rawliteral(
<style>
  body { background-color: #ffffff; color: #000000; font-family: sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; margin: 0; min-height: 100vh; }
  h1 { color: #2E6F40; margin-bottom: 20px; font-size: 2.2em; }
  .wrap { background-color: #f9f9f9; padding: 30px; border-radius: 15px; border: 1px solid #ddd; width: 90%; max-width: 380px; box-sizing: border-box; }
  button { background-color: #2E6F40; color: white; border: none; border-radius: 25px; padding: 15px; width: 100%; font-weight: bold; cursor: pointer; margin-top: 20px; }
  input { border: 1px solid #2E6F40; border-radius: 8px; padding: 12px; width: 100%; box-sizing: border-box; }
</style>
)rawliteral";

void showStatus(const char* line1, const char* line2 = "") {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("SPROUTY SMART SENSOR");
  display.println("--------------------");
  display.println("");
  display.println(line1);
  display.println("");
  display.println(line2);
  display.display();
}

void connectWiFi() {
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    showStatus("Resetting WiFi...");

    WiFiManager wm;
    wm.resetSettings();

    rtc_valid = false;
    delay(2000);
  }

  WiFi.mode(WIFI_STA);
  if (rtc_valid) {
    WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str(), rtc_channel, rtc_bssid);
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 25) {
      delay(100);
      timeout++;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    showStatus("WiFi Setup Needed", "Connect to WiFi named\nSprouty-Setup");

    WiFiManager wm;
    wm.setCustomHeadElement(SPROUTY_CSS);
    wm.setTitle("Sprouty Setup");
    std::vector<const char *> menu = {"wifi"}; 
    wm.setMenu(menu);
    if (!wm.autoConnect("Sprouty-Setup")) {
      ESP.restart();
    }
  }

  rtc_channel = WiFi.channel();
  memcpy(rtc_bssid, WiFi.BSSID(), 6);
  rtc_valid = true;
}

void sendSensorData() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  int moisturePercent = map(analogRead(SOIL_PIN), 4095, 2415, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  if (isnan(hum) || isnan(temp)) {
    hum = 0.0;
    temp = 0.0;
  }

  HTTPClient http;
  http.begin(SERVER_URL_DATA); 
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"sensorId\":\"" + deviceID + "\",\"moisture\":" + String(moisturePercent) + ",\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + "}";
  http.POST(payload);
  http.end();
}

void sendImageData() {
  showStatus("Noon Photo!");
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  if(esp_camera_init(&config) != ESP_OK) return;
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 1);
  s->set_ae_level(s, 1);
  s->set_whitebal(s, 1);
  s->set_wb_mode(s, 0);

  delay(2000); 

  for(int i = 0; i < 3; i++) {
    camera_fb_t* fb_junk = esp_camera_fb_get();
    if (fb_junk) esp_camera_fb_return(fb_junk);
    delay(200);
  }

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) return;

  WiFiClient client;
  if (client.connect(SERVER_IP, SERVER_PORT)) {
    String boundary = "--------------------------ESP32Boundary";
    String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"sensorId\"\r\n\r\n" + deviceID + "\r\n" +
                  "--" + boundary + "\r\nContent-Disposition: form-data; name=\"image\"; filename=\"p.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--" + boundary + "--\r\n";
    client.print("POST " + String(SERVER_PATH_IMAGE) + " HTTP/1.1\r\nHost: " + String(SERVER_IP) + "\r\nContent-Type: multipart/form-data; boundary=" + boundary + "\r\nContent-Length: " + String(head.length() + fb->len + tail.length()) + "\r\nConnection: close\r\n\r\n");
    client.print(head); client.write(fb->buf, fb->len); client.print(tail);
    delay(1000); client.stop();
  }
  esp_camera_fb_return(fb);
  esp_camera_deinit();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  dht.begin();

  deviceID = WiFi.macAddress();
  deviceID.replace(":", "");

  connectWiFi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) Serial.println("Time Fail");

  sendSensorData();

  uint64_t sleepUS = DATA_INTERVAL_HOURS * uS_TO_HOUR;
  if (timeinfo.tm_hour == 12) {
    sendImageData();
  } else if (timeinfo.tm_hour >= 6 && timeinfo.tm_hour < 12) {
    int minsToNoon = ((12 - timeinfo.tm_hour) * 60) - timeinfo.tm_min;
    uint64_t usToNoon = (uint64_t)minsToNoon * 60 * 1000000ULL;
    if (usToNoon < sleepUS) sleepUS = usToNoon;
  }

  int waitMins = (int)(sleepUS / 60000000ULL);
  char buf[32];
  sprintf(buf, "Next: %dh %dm", waitMins / 60, waitMins % 60);
  showStatus("Sync Complete", buf);
  
  delay(5000);
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  esp_sleep_enable_timer_wakeup(sleepUS);
  esp_deep_sleep_start();
}

void loop() {}
