/*
 * ESP32 Dash Cam with AI
 * AI-powered dash cam with accident detection and cloud storage
 * 
 * Components:
 * - Camera OV2640
 * - SD Card
 * - GPS Module
 * - Accelerometer (MPU6050)
 * - Power Supply 12V
 * 
 * Libraries Required:
 * - ESP32 Camera Library
 * - SD Library
 * - TinyGPS++ Library
 * - MPU6050 Library
 */

#include "esp_camera.h"
#include <SD.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <MPU6050.h>

// ===== Camera Pins =====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ===== Objects =====
TinyGPSPlus gps;
MPU6050 mpu;
WiFiClient client;

// ===== Variables =====
bool recording = false;
bool accidentDetected = false;
unsigned long lastFrame = 0;
int frameCount = 0;
float gForceX, gForceY, gForceZ;
float accelThreshold = 3.0;

// ===== Setup =====
void setup() {
    Serial.begin(115200);
    
    // Initialize Camera
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
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    
    // Initialize SD Card
    if (!SD.begin(4)) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    
    // Initialize MPU6050
    Wire.begin();
    mpu.initialize();
    
    Serial.println("Dash Cam Ready!");
}

// ===== Main Loop =====
void loop() {
    // Read GPS
    while (Serial2.available() > 0) {
        gps.encode(Serial2.read());
    }
    
    // Read Accelerometer
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    gForceX = ax / 16384.0;
    gForceY = ay / 16384.0;
    gForceZ = az / 16384.0;
    
    // Check for accident
    float totalG = sqrt(gForceX*gForceX + gForceY*gForceY + gForceZ*gForceZ);
    if (totalG > accelThreshold) {
        accidentDetected = true;
        handleAccident();
    }
    
    // Record video
    if (recording) {
        captureFrame();
    }
    
    delay(50);
}

// ===== Capture Frame =====
void captureFrame() {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    
    // Save to SD
    String filename = "/dashcam_" + String(millis()) + ".jpg";
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        file.write(fb->buf, fb->len);
        file.close();
        frameCount++;
    }
    
    esp_camera_fb_return(fb);
}

// ===== Handle Accident =====
void handleAccident() {
    Serial.println("🚨 ACCIDENT DETECTED!");
    
    // Capture and save image
    camera_fb_t * fb = esp_camera_fb_get();
    if (fb) {
        String filename = "/accident_" + String(millis()) + ".jpg";
        File file = SD.open(filename, FILE_WRITE);
        if (file) {
            file.write(fb->buf, fb->len);
            file.close();
        }
        esp_camera_fb_return(fb);
    }
    
    // Send alert with location
    if (gps.location.isValid()) {
        sendAlert("Accident detected at " + 
                  String(gps.location.lat(), 6) + "," + 
                  String(gps.location.lng(), 6));
    }
    
    // Upload to cloud
    uploadToCloud();
    
    accidentDetected = false;
}

// ===== Send Alert =====
void sendAlert(String message) {
    Serial.println("Alert: " + message);
    // Send via WiFi/LTE
}

// ===== Upload to Cloud =====
void uploadToCloud() {
    Serial.println("Uploading to cloud...");
    // Upload files to cloud storage
}

// ===== Start/Stop Recording =====
void toggleRecording() {
    recording = !recording;
    if (recording) {
        Serial.println("Recording started");
    } else {
        Serial.println("Recording stopped");
    }
}

// ===== Get GPS Data =====
String getGPSData() {
    if (gps.location.isValid()) {
        return String(gps.location.lat(), 6) + "," + 
               String(gps.location.lng(), 6) + "," +
               String(gps.speed.kmph(), 1);
    }
    return "No GPS";
}
