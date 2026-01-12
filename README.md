# Sprouty Smart Sensor — IoT Firmware

![C++](https://img.shields.io/badge/C++-Solutions-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-ESP32--S3--CAM-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)

This repository contains the firmware for the **Sprouty Smart Sensor**. The device acts as the "eyes and ears" of the Sprouty ecosystem, capturing environmental telemetry and plant imagery.

---

## Key Features

* **Deep Sleep Optimization:** Consumes minimal power by waking up every 6 hours for data sync.
* **Fast WiFi Reconnect:** Utilizes RTC memory to store WiFi channel and BSSID, reducing connection time and battery drain.
* **Intelligent Scheduling:** Syncs with NTP servers to take high-quality "Noon Photos" for consistent lighting in time-lapses.
* **Custom Captive Portal:** Powered by `WiFiManager` with a custom-branded CSS interface for seamless user onboarding.
* **Multi-Sensor Data Ingestion:**
    * **Air:** DHT11 for temperature and humidity.
    * **Soil:** Analog capacitive moisture sensing.
    * **Vision:** OV2640 camera for JPEG image payloads.

---

## Hardware Specifications

| Component | Specification |
| :--- | :--- |
| **Microcontroller** | ESP32-S3-CAM |
| **Display** | SSD1306 OLED (128x64) via I2C |
| **Sensors** | DHT11 (Temp/Hum), Analog Moisture Probe |
| **Camera** | OV2640 (SVGA Resolution) |
| **Communication** | WiFi 802.11 b/g/n |



---

## Data Flow & API Integration

The firmware communicates with the **Sprouty Backend** via two primary endpoints:

1.  **Telemetry (`POST /sensors/data`):** Sends a JSON payload containing `moisture`, `temperature`, and `humidity` mapped to the device's MAC Address.
2.  **Imaging (`POST /sensors/image`):** Sends a `multipart/form-data` request containing the JPEG binary and the `sensorId`.

### Tokenization
The `sensorId` is generated automatically from the hardware MAC address (e.g., `AABBCCDDEEFF`), ensuring a unique identifier for every device without manual configuration.

---

## Installation & Setup

### Prerequisites
* **Arduino IDE**
* **Libraries:** * `Adafruit_SSD1306`
    * `DHT sensor library`
    * `WiFiManager`
    * `Espressif ESP32 Camera`

### Specifics
#### Make sure to use these settings in you Arduino IDE - Tools
* **USB CDC On Boot:** Enabled
* **Erase all Flash before sketch upload:** Enabled
* **Flash Mode:** DIO 80MHz
* **Flash Size:** 16MB
* **Partition Scheme:** Default 4MB with spiffs(1.2MB APP/!.5MB SPIFFS)
* **PSRAM:** OPI PSRAM
 

### Deployment
1.  **Configure Host:** Update `SERVER_HOST` in the code to your domain (default: `sprouty.duckdns.org`).
2.  **Flash:** Upload to your ESP32-S3-CAM board.
3.  **Onboarding:**
    * On first boot, connect to the **"Sprouty-Setup"** WiFi hotspot.
    * Enter your home WiFi credentials in the branded web portal.
    * The device will sync and enter Deep Sleep automatically.

---

## Pin Mapping

| Peripheral | ESP32 Pin |
| :--- | :--- |
| **Soil Sensor** | GPIO 1 |
| **DHT11** | GPIO 2 |
| **OLED SDA** | GPIO 42 |
| **OLED SCL** | GPIO 41 |
| **Boot Button**| GPIO 0 |

---

**Author:** David Muhič  
**Project:** Sprouty — Smart Plant Companion  
**Course:** PRPO 2025/2026
