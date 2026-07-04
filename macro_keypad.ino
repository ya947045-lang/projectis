/*
 * Pro Micro Macro Keypad Pro
 * 12-key mechanical keypad with 4 rotary encoders
 * 
 * Components:
 * - 12x Cherry MX Switches
 * - 4x Rotary Encoders
 * - OLED 0.96" Display
 * - LEDs
 * 
 * Libraries Required:
 * - Keypad Library
 * - Adafruit SSD1306 Library
 * - Encoder Library
 */

#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>

// ===== Keypad Configuration =====
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ===== Rotary Encoders =====
Encoder enc1(10, 11);
Encoder enc2(12, 13);
Encoder enc3(14, 15);
Encoder enc4(16, 17);

// ===== OLED Display =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Variables =====
int encoderValues[4] = {0, 0, 0, 0};
long oldPos[4] = {-999, -999, -999, -999};
String currentProfile = "Default";
String keyLabels[12] = {"F1", "F2", "F3", "F4", "F5", "F6", 
                        "F7", "F8", "F9", "F10", "F11", "F12"};

// ===== Setup =====
void setup() {
    Serial.begin(9600);
    
    // Initialize OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("OLED not found!");
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Macro Keypad Pro");
    display.println("Ready!");
    display.display();
    
    // Initialize as keyboard
    Keyboard.begin();
}

// ===== Main Loop =====
void loop() {
    // Check keypad
    char key = keypad.getKey();
    if (key) {
        handleKey(key);
    }
    
    // Check rotary encoders
    readEncoders();
    
    // Update display
    updateDisplay();
    
    delay(10);
}

// ===== Handle Key Press =====
void handleKey(char key) {
    int index = -1;
    
    // Convert key to index
    switch(key) {
        case '1': index = 0; break;
        case '2': index = 1; break;
        case '3': index = 2; break;
        case '4': index = 3; break;
        case '5': index = 4; break;
        case '6': index = 5; break;
        case '7': index = 6; break;
        case '8': index = 7; break;
        case '9': index = 8; break;
        case '*': index = 9; break;
        case '0': index = 10; break;
        case '#': index = 11; break;
    }
    
    if (index != -1) {
        // Send key press
        Keyboard.press(keyLabels[index]);
        delay(100);
        Keyboard.release(keyLabels[index]);
        
        // Update display
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Key Pressed:");
        display.println(keyLabels[index]);
        display.display();
        delay(500);
    }
}

// ===== Read Encoders =====
void readEncoders() {
    long newPos[4];
    newPos[0] = enc1.read();
    newPos[1] = enc2.read();
    newPos[2] = enc3.read();
    newPos[3] = enc4.read();
    
    for (int i = 0; i < 4; i++) {
        if (newPos[i] != oldPos[i]) {
            int diff = (newPos[i] - oldPos[i]) / 4;
            encoderValues[i] += diff;
            oldPos[i] = newPos[i];
            
            // Send value as key
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.print(encoderValues[i]);
            delay(10);
            Keyboard.releaseAll();
        }
    }
}

// ===== Update Display =====
void updateDisplay() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < 1000) return;
    lastUpdate = millis();
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    
    display.print("Profile: ");
    display.println(currentProfile);
    
    display.print("E1:");
    display.print(encoderValues[0]);
    display.print(" E2:");
    display.println(encoderValues[1]);
    
    display.print("E3:");
    display.print(encoderValues[2]);
    display.print(" E4:");
    display.println(encoderValues[3]);
    
    display.display();
}

// ===== Change Profile =====
void setProfile(String profile) {
    currentProfile = profile;
    // Load key mappings for profile
    if (profile == "Gaming") {
        String keys[12] = {"W", "A", "S", "D", "Q", "E", 
                          "R", "F", "G", "T", "Y", "U"};
        for (int i = 0; i < 12; i++) {
            keyLabels[i] = keys[i];
        }
    } else if (profile == "Video") {
        String keys[12] = {"Space", "←", "→", "↑", "↓", "Ctrl+Z", 
                          "Ctrl+X", "Ctrl+C", "Ctrl+V", "Enter", "Delete", "Esc"};
        for (int i = 0; i < 12; i++) {
            keyLabels[i] = keys[i];
        }
    }
}
