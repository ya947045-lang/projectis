/*
 * ESP32 Smart Plant Watering
 * Smart plant watering system with soil moisture sensors and solar power
 * 
 * Components:
 * - Soil Moisture Sensor
 * - Water Pump
 * - Solar Panel 5V
 * - Battery 18650
 * - Relay Module
 * 
 * Libraries Required:
 * - WiFi Library
 * - HTTPClient Library
 * - ArduinoJson Library
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===== WiFi Credentials =====
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ===== Pin Definitions =====
#define SOIL_SENSOR_PIN 34
#define WATER_PUMP_RELAY 5
#define BATTERY_LEVEL_PIN 35
#define LIGHT_SENSOR_PIN 32

// ===== Constants =====
const int MOISTURE_THRESHOLD = 500;
const unsigned long WATERING_INTERVAL = 3600000; // 1 hour
const unsigned long READING_INTERVAL = 60000; // 1 minute

// ===== Variables =====
int soilMoisture = 0;
int batteryLevel = 0;
int lightLevel = 0;
bool isWatering = false;
unsigned long lastWatering = 0;
unsigned long lastReading = 0;
int wateringDuration = 5000; // 5 seconds
bool solarPowered = true;

// ===== Setup =====
void setup() {
    Serial.begin(115200);
    
    // Initialize pins
    pinMode(SOIL_SENSOR_PIN, INPUT);
    pinMode(WATER_PUMP_RELAY, OUTPUT);
    pinMode(BATTERY_LEVEL_PIN, INPUT);
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    
    digitalWrite(WATER_PUMP_RELAY, HIGH); // Relay off
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("WiFi Connected!");
    
    Serial.println("Plant Watering System Ready!");
}

// ===== Main Loop =====
void loop() {
    if (millis() - lastReading >= READING_INTERVAL) {
        readSensors();
        checkWatering();
        sendData();
        lastReading = millis();
    }
    
    // Control watering
    if (isWatering) {
        if (millis() - lastWatering >= wateringDuration) {
            stopWatering();
        }
    }
    
    delay(100);
}

// ===== Read Sensors =====
void readSensors() {
    soilMoisture = analogRead(SOIL_SENSOR_PIN);
    batteryLevel = analogRead(BATTERY_LEVEL_PIN);
    lightLevel = analogRead(LIGHT_SENSOR_PIN);
    
    // Convert to percentage
    int moisturePercent = map(soilMoisture, 0, 4095, 0, 100);
    int batteryPercent = map(batteryLevel, 0, 4095, 0, 100);
    
    Serial.println("Soil Moisture: " + String(moisturePercent) + "%");
    Serial.println("Battery: " + String(batteryPercent) + "%");
    Serial.println("Light: " + String(lightLevel));
}

// ===== Check Watering =====
void checkWatering() {
    // Check if soil is dry
    if (soilMoisture > MOISTURE_THRESHOLD) {
        // Check if enough time has passed since last watering
        if (millis() - lastWatering >= WATERING_INTERVAL) {
            // Check if battery has enough charge (if solar powered)
            if (solarPowered && batteryLevel < 2000) {
                Serial.println("Battery too low, waiting for solar charge");
                return;
            }
            
            startWatering();
        }
    } else {
        Serial.println("Soil moisture is adequate");
    }
}

// ===== Start Watering =====
void startWatering() {
    isWatering = true;
    digitalWrite(WATER_PUMP_RELAY, LOW); // Turn on pump
    lastWatering = millis();
    
    Serial.println("💧 Watering plant...");
    
    // Send notification
    if (WiFi.status() == WL_CONNECTED) {
        sendNotification("🌱 Plant watering started");
    }
}

// ===== Stop Watering =====
void stopWatering() {
    isWatering = false;
    digitalWrite(WATER_PUMP_RELAY, HIGH); // Turn off pump
    
    Serial.println("✅ Watering complete");
    
    // Send notification
    if (WiFi.status() == WL_CONNECTED) {
        sendNotification("✅ Plant watering complete");
    }
}

// ===== Send Data to Cloud =====
void sendData() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin("http://your-server.com/api/plant");
    http.addHeader("Content-Type", "application/json");
    
    StaticJsonDocument<200> doc;
    doc["moisture"] = soilMoisture;
    doc["battery"] = batteryLevel;
    doc["light"] = lightLevel;
    doc["watering"] = isWatering;
    
    String json;
    serializeJson(doc, json);
    
    int httpCode = http.POST(json);
    http.end();
}

// ===== Send Notification =====
void sendNotification(String message) {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin("http://your-server.com/api/notification");
    http.addHeader("Content-Type", "application/json");
    
    StaticJsonDocument<200> doc;
    doc["message"] = message;
    doc["type"] = "plant_watering";
    
    String json;
    serializeJson(doc, json);
    http.POST(json);
    http.end();
}

// ===== Manual Water =====
void manualWater(int duration) {
    wateringDuration = duration * 1000;
    if (!isWatering) {
        startWatering();
    }
}

// ===== Check Solar Power =====
void checkSolarPower() {
    // Check if solar panel is producing enough power
    if (lightLevel > 1000) {
        // Solar power available
        solarPowered = true;
        // Charge battery
        Serial.println("☀️ Solar power available, charging battery");
    } else {
        solarPowered = false;
        Serial.println("🌙 No solar power, using battery");
    }
}
