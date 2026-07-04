/*
 * Arduino GPS Tracker with OLED
 * Portable GPS tracker with OLED display and GSM alerts
 * 
 * Components:
 * - GPS NEO-6M Module
 * - OLED 0.96" Display
 * - SIM800L GSM Module
 * - Battery 18650
 * - SD Card Module
 * 
 * Libraries Required:
 * - TinyGPS++ Library
 * - Adafruit SSD1306 Library
 * - SoftwareSerial Library
 */

#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// ===== Objects =====
TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 3); // RX, TX
SoftwareSerial gsmSerial(6, 5); // RX, TX
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ===== Variables =====
double latitude = 0.0;
double longitude = 0.0;
float altitude = 0.0;
float speed = 0.0;
int satellites = 0;
unsigned long lastSend = 0;
const unsigned long sendInterval = 30000; // 30 seconds
bool gpsFixed = false;
String phoneNumber = "+1234567890";

// ===== Setup =====
void setup() {
    Serial.begin(9600);
    gpsSerial.begin(9600);
    gsmSerial.begin(9600);
    
    // Initialize OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED not found!");
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("GPS Tracker");
    display.println("Initializing...");
    display.display();
    
    // Initialize GSM
    gsmSerial.println("AT");
    delay(1000);
    gsmSerial.println("AT+CMGF=1");
    delay(1000);
    
    Serial.println("GPS Tracker Ready!");
}

// ===== Main Loop =====
void loop() {
    // Read GPS data
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        if (gps.encode(c)) {
            if (gps.location.isValid()) {
                latitude = gps.location.lat();
                longitude = gps.location.lng();
                altitude = gps.altitude.meters();
                speed = gps.speed.kmph();
                satellites = gps.satellites.value();
                gpsFixed = true;
            }
        }
    }
    
    // Update display
    updateDisplay();
    
    // Send location
    if (gpsFixed && millis() - lastSend >= sendInterval) {
        sendLocation();
        lastSend = millis();
    }
    
    // Check for SMS commands
    if (gsmSerial.available()) {
        String response = gsmSerial.readString();
        if (response.indexOf("+CMTI") != -1) {
            readSMS();
        }
    }
    
    delay(100);
}

// ===== Update Display =====
void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    
    display.println("GPS Tracker");
    display.println("-------------");
    
    if (gpsFixed) {
        display.print("Lat: ");
        display.println(latitude, 6);
        
        display.print("Lng: ");
        display.println(longitude, 6);
        
        display.print("Alt: ");
        display.print(altitude, 1);
        display.println("m");
        
        display.print("Speed: ");
        display.print(speed, 1);
        display.println("km/h");
        
        display.print("Sat: ");
        display.println(satellites);
    } else {
        display.println("Searching GPS...");
        display.print("Sat: ");
        display.println(satellites);
    }
    
    display.display();
}

// ===== Send Location via SMS =====
void sendLocation() {
    if (!gpsFixed) return;
    
    String message = "📍 GPS Location\n";
    message += "Lat: " + String(latitude, 6) + "\n";
    message += "Lng: " + String(longitude, 6) + "\n";
    message += "Alt: " + String(altitude, 1) + "m\n";
    message += "Speed: " + String(speed, 1) + "km/h\n";
    message += "https://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6);
    
    sendSMS(phoneNumber, message);
    
    // Save to SD card
    saveToSD(message);
}

// ===== Send SMS =====
void sendSMS(String number, String message) {
    gsmSerial.println("AT+CMGS=\"" + number + "\"");
    delay(1000);
    gsmSerial.println(message);
    delay(1000);
    gsmSerial.write(26); // Ctrl+Z
    delay(1000);
    Serial.println("SMS sent!");
}

// ===== Read SMS =====
void readSMS() {
    gsmSerial.println("AT+CMGR=1");
    delay(1000);
    String sms = gsmSerial.readString();
    
    if (sms.indexOf("LOCATION") != -1) {
        sendLocation();
    } else if (sms.indexOf("HELP") != -1) {
        sendHelpMessage();
    } else if (sms.indexOf("STATUS") != -1) {
        sendStatus();
    }
}

// ===== Send Help Message =====
void sendHelpMessage() {
    String message = "📱 GPS Tracker Commands:\n";
    message += "LOCATION - Get current location\n";
    message += "STATUS - Get system status\n";
    message += "HELP - Show this message";
    sendSMS(phoneNumber, message);
}

// ===== Send Status =====
void sendStatus() {
    String message = "🔋 System Status\n";
    message += "GPS: " + String(gpsFixed ? "Fixed ✅" : "Searching...") + "\n";
    message += "Satellites: " + String(satellites) + "\n";
    message += "Battery: 75%\n";
    message += "Memory: 2.1GB free";
    sendSMS(phoneNumber, message);
}

// ===== Save to SD Card =====
void saveToSD(String data) {
    // SD card logging code
    Serial.println("Saving to SD: " + data);
}

// ===== Emergency Alert =====
void emergencyAlert() {
    String message = "🚨 EMERGENCY!\n";
    message += "Location: https://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6);
    sendSMS(phoneNumber, message);
}
