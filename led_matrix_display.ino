/*
 * Pico LED Matrix Display
 * 64x32 RGB LED matrix display showing time, weather, and animations
 * 
 * Components:
 * - LED Matrix 64x32
 * - Power Supply 5V
 * - RTC Module (DS3231)
 * 
 * Libraries Required:
 * - Adafruit GFX Library
 * - Adafruit LED Backpack Library
 * - RTClib Library
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <RTClib.h>

// ===== Objects =====
Adafruit_Matrix matrix = Adafruit_Matrix(64, 32);
RTC_DS3231 rtc;

// ===== Variables =====
String messages[] = {
    "Future Maker",
    "Hello World!",
    "Build the Future"
};
int messageIndex = 0;
int scrollPosition = 0;
unsigned long lastUpdate = 0;
int displayMode = 0; // 0=clock, 1=weather, 2=message

// ===== Setup =====
void setup() {
    Serial.begin(9600);
    
    // Initialize matrix
    matrix.begin(0x70);
    matrix.setBrightness(8);
    matrix.clear();
    matrix.writeDisplay();
    
    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("RTC not found!");
    }
    
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    Serial.println("LED Matrix Ready!");
}

// ===== Main Loop =====
void loop() {
    if (millis() - lastUpdate >= 1000) {
        updateDisplay();
        lastUpdate = millis();
    }
    
    delay(10);
}

// ===== Update Display =====
void updateDisplay() {
    matrix.clear();
    matrix.setTextSize(2);
    matrix.setTextColor(LED_RED);
    
    switch(displayMode) {
        case 0:
            displayClock();
            break;
        case 1:
            displayWeather();
            break;
        case 2:
            displayScrollingMessage();
            break;
        case 3:
            displayAnimation();
            break;
    }
    
    matrix.writeDisplay();
}

// ===== Display Clock =====
void displayClock() {
    DateTime now = rtc.now();
    
    String timeStr = String(now.hour()) + ":" + 
                     String(now.minute()) + ":" + 
                     String(now.second());
    
    matrix.setCursor(0, 8);
    matrix.setTextColor(LED_RED);
    matrix.print(timeStr);
    
    // Display date
    matrix.setTextSize(1);
    matrix.setCursor(0, 24);
    matrix.setTextColor(LED_GREEN);
    String dateStr = String(now.day()) + "/" + 
                     String(now.month()) + "/" + 
                     String(now.year());
    matrix.print(dateStr);
}

// ===== Display Weather =====
void displayWeather() {
    // Simulated weather data
    int temperature = 25;
    int humidity = 60;
    
    matrix.setTextSize(1);
    matrix.setCursor(0, 0);
    matrix.setTextColor(LED_RED);
    matrix.print("Temp:");
    matrix.print(temperature);
    matrix.print("C");
    
    matrix.setCursor(0, 16);
    matrix.setTextColor(LED_GREEN);
    matrix.print("Humidity:");
    matrix.print(humidity);
    matrix.print("%");
}

// ===== Display Scrolling Message =====
void displayScrollingMessage() {
    String msg = messages[messageIndex];
    
    int textWidth = msg.length() * 6;
    matrix.setTextSize(1);
    matrix.setTextColor(LED_YELLOW);
    matrix.setCursor(-scrollPosition, 12);
    matrix.print(msg);
    
    scrollPosition++;
    if (scrollPosition > textWidth + 64) {
        scrollPosition = 0;
        messageIndex = (messageIndex + 1) % 3;
    }
}

// ===== Display Animation =====
void displayAnimation() {
    static int frame = 0;
    static int x = -8, y = 10;
    
    matrix.clear();
    
    // Draw a bouncing ball
    matrix.fillCircle(x, y, 4, LED_RED);
    
    x += 2;
    if (x > 64) x = -8;
    
    y += 1;
    if (y > 32) y = 10;
    
    // Draw some stars
    matrix.drawPixel(10 + frame, 5, LED_GREEN);
    matrix.drawPixel(30 + frame, 8, LED_GREEN);
    matrix.drawPixel(50 - frame, 5, LED_GREEN);
    
    // Draw text
    matrix.setTextSize(1);
    matrix.setCursor(20, 25);
    matrix.setTextColor(LED_BLUE);
    matrix.print("Future");
    
    frame = (frame + 1) % 64;
}

// ===== Set Brightness =====
void setBrightness(int brightness) {
    brightness = constrain(brightness, 0, 15);
    matrix.setBrightness(brightness);
    matrix.writeDisplay();
}

// ===== Switch Mode =====
void switchMode(int mode) {
    displayMode = mode % 4;
    scrollPosition = 0;
}

// ===== Set Message =====
void setMessage(String message) {
    messages[0] = message;
}

// ===== Display Status =====
void displayStatus(String status) {
    matrix.clear();
    matrix.setTextSize(1);
    matrix.setCursor(0, 12);
    matrix.setTextColor(LED_WHITE);
    matrix.print(status);
    matrix.writeDisplay();
}
