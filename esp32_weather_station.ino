/*
 * ESP32 Weather Station Pro
 * Complete weather station with OLED display, sensors, and web dashboard
 * 
 * Components:
 * - DHT22 (Temperature & Humidity)
 * - BMP280 (Pressure & Altitude)
 * - OLED 128x64 (Display)
 * - MQ-135 (Air Quality)
 * - Anemometer (Wind Speed)
 * - Rain Sensor
 * 
 * Libraries Required:
 * - Adafruit Sensor Library
 * - DHT Sensor Library
 * - Adafruit BMP280 Library
 * - Adafruit SSD1306 Library
 * - WiFi Library
 */

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>

// ===== WiFi Credentials =====
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ===== Pin Definitions =====
#define DHTPIN 4
#define DHTTYPE DHT22
#define RAIN_PIN 34
#define ANEMOMETER_PIN 35
#define MQ135_PIN 32

// ===== OLED Display =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

// ===== Objects =====
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Variables =====
float temperature, humidity, pressure, altitude;
float airQuality, windSpeed;
int rainValue;
unsigned long lastReading = 0;
const unsigned long readingInterval = 5000;

// ===== Setup =====
void setup() {
    Serial.begin(115200);
    
    // Initialize sensors
    dht.begin();
    
    if (!bmp.begin(0x76)) {
        Serial.println("BMP280 not found!");
    }
    
    // Initialize OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("OLED not found!");
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Weather Station");
    display.println("Initializing...");
    display.display();
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected!");
}

// ===== Main Loop =====
void loop() {
    if (millis() - lastReading >= readingInterval) {
        readSensors();
        updateDisplay();
        sendToServer();
        lastReading = millis();
    }
    delay(100);
}

// ===== Read Sensors =====
void readSensors() {
    // Read DHT22
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    
    // Read BMP280
    pressure = bmp.readPressure() / 100.0F;
    altitude = bmp.readAltitude(1013.25);
    
    // Read Analog Sensors
    airQuality = analogRead(MQ135_PIN);
    windSpeed = analogRead(ANEMOMETER_PIN) * 0.1;
    rainValue = analogRead(RAIN_PIN);
}

// ===== Update Display =====
void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
    
    display.setTextSize(1);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");
    
    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");
    
    display.print("Pressure: ");
    display.print(pressure);
    display.println(" hPa");
    
    display.print("Air Quality: ");
    display.println(airQuality);
    
    display.print("Wind: ");
    display.print(windSpeed);
    display.println(" m/s");
    
    display.display();
}

// ===== Send to Server =====
void sendToServer() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("http://your-server.com/api/weather");
        http.addHeader("Content-Type", "application/json");
        
        String json = "{\"temperature\":" + String(temperature) + 
                     ",\"humidity\":" + String(humidity) +
                     ",\"pressure\":" + String(pressure) +
                     ",\"airQuality\":" + String(airQuality) +
                     ",\"windSpeed\":" + String(windSpeed) + "}";
        
        int httpCode = http.POST(json);
        http.end();
    }
}
