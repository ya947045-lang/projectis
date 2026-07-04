/*
 * Arduino Home Security System
 * Complete security with camera, motion sensors, and Telegram notifications
 * 
 * Components:
 * - PIR Sensor (Motion Detection)
 * - OV7670 Camera
 * - Buzzer (Alarm)
 * - LED Strip
 * - Relay Module
 * 
 * Libraries Required:
 * - ArduinoHttpClient Library
 * - WiFi Library (for ESP8266)
 * - Servo Library
 */

#include <Servo.h>
#include <Wire.h>
#include <SoftwareSerial.h>

// ===== Pin Definitions =====
#define PIR_PIN 2
#define BUZZER_PIN 3
#define RELAY_PIN 4
#define LED_PIN 5
#define CAMERA_ENABLE 6

// ===== Objects =====
Servo doorLock;
SoftwareSerial espSerial(10, 11); // RX, TX

// ===== Variables =====
bool motionDetected = false;
bool alarmActive = false;
bool systemArmed = true;
unsigned long lastMotionTime = 0;
const unsigned long motionTimeout = 5000;
String lastTelegramMessage = "";

// ===== Setup =====
void setup() {
    Serial.begin(9600);
    espSerial.begin(115200);
    
    // Pin modes
    pinMode(PIR_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(CAMERA_ENABLE, OUTPUT);
    
    // Attach servo
    doorLock.attach(9);
    doorLock.write(0); // Locked position
    
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(CAMERA_ENABLE, HIGH);
    
    Serial.println("Security System Ready!");
    sendTelegramMessage("✅ Security System Activated!");
}

// ===== Main Loop =====
void loop() {
    // Check PIR sensor
    int pirValue = digitalRead(PIR_PIN);
    
    if (pirValue == HIGH && systemArmed) {
        if (!motionDetected) {
            motionDetected = true;
            lastMotionTime = millis();
            
            Serial.println("🚨 MOTION DETECTED!");
            
            // Trigger alarm
            triggerAlarm();
            
            // Send notification
            sendTelegramMessage("🚨 MOTION DETECTED! Intruder alert!");
            
            // Take photo (simulated)
            takePhoto();
        }
    } else if (motionDetected && (millis() - lastMotionTime > motionTimeout)) {
        motionDetected = false;
        resetAlarm();
        Serial.println("Motion cleared");
    }
    
    // Check for ESP commands
    if (espSerial.available()) {
        String command = espSerial.readStringUntil('\n');
        handleCommand(command);
    }
    
    delay(100);
}

// ===== Trigger Alarm =====
void triggerAlarm() {
    alarmActive = true;
    
    // Sound buzzer
    for (int i = 0; i < 5; i++) {
        tone(BUZZER_PIN, 1000);
        digitalWrite(LED_PIN, HIGH);
        delay(300);
        noTone(BUZZER_PIN);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
    
    // Turn on relay (siren)
    digitalWrite(RELAY_PIN, HIGH);
}

// ===== Reset Alarm =====
void resetAlarm() {
    alarmActive = false;
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
}

// ===== Take Photo =====
void takePhoto() {
    digitalWrite(CAMERA_ENABLE, HIGH);
    delay(100);
    // Camera capture code would go here
    digitalWrite(CAMERA_ENABLE, LOW);
    
    // Send to ESP8266 for cloud upload
    espSerial.println("PHOTO:capture");
}

// ===== Handle Commands =====
void handleCommand(String command) {
    command.trim();
    
    if (command == "ARM") {
        systemArmed = true;
        doorLock.write(0);
        sendTelegramMessage("🔒 System Armed");
        Serial.println("System Armed");
    } else if (command == "DISARM") {
        systemArmed = false;
        doorLock.write(90);
        resetAlarm();
        sendTelegramMessage("🔓 System Disarmed");
        Serial.println("System Disarmed");
    } else if (command == "STATUS") {
        sendStatus();
    } else if (command.startsWith("DOOR:")) {
        int angle = command.substring(5).toInt();
        doorLock.write(angle);
    } else if (command == "PHOTO") {
        takePhoto();
    }
}

// ===== Send Status =====
void sendStatus() {
    String status = "🏠 Security Status\n";
    status += "Armed: " + String(systemArmed ? "✅" : "❌") + "\n";
    status += "Motion: " + String(motionDetected ? "🚨" : "✅") + "\n";
    status += "Alarm: " + String(alarmActive ? "🔊" : "🔇") + "\n";
    status += "Door: " + String(doorLock.read() > 45 ? "Unlocked" : "Locked");
    sendTelegramMessage(status);
}

// ===== Send Telegram Message =====
void sendTelegramMessage(String message) {
    espSerial.println("TELEGRAM:" + message);
}

// ===== Send to ESP8266 =====
void sendToESP(String data) {
    espSerial.println(data);
    delay(100);
}
