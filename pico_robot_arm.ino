/*
 * Raspberry Pi Pico Robot Arm
 * 6-axis robotic arm with Bluetooth control
 * 
 * Components:
 * - 6x Servo Motors (SG90/MG995)
 * - HC-05 Bluetooth Module
 * - Joystick Module
 * - Power Supply 5V 5A
 * 
 * Libraries Required:
 * - Servo Library
 * - SoftwareSerial Library
 */

#include <Servo.h>
#include <SoftwareSerial.h>

// ===== Pin Definitions =====
#define SERVO1_PIN 2
#define SERVO2_PIN 3
#define SERVO3_PIN 4
#define SERVO4_PIN 5
#define SERVO5_PIN 6
#define SERVO6_PIN 7

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_BTN 8

#define BLUETOOTH_TX 0
#define BLUETOOTH_RX 1

// ===== Objects =====
Servo servo1, servo2, servo3, servo4, servo5, servo6;
SoftwareSerial bluetooth(BLUETOOTH_RX, BLUETOOTH_TX);

// ===== Variables =====
int servoPositions[6] = {90, 90, 90, 90, 90, 90};
int targetPositions[6] = {90, 90, 90, 90, 90, 90};
bool autoMode = false;
unsigned long lastCommand = 0;

// ===== Setup =====
void setup() {
    Serial.begin(9600);
    bluetooth.begin(9600);
    
    // Attach servos
    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);
    servo3.attach(SERVO3_PIN);
    servo4.attach(SERVO4_PIN);
    servo5.attach(SERVO5_PIN);
    servo6.attach(SERVO6_PIN);
    
    // Set initial positions
    for (int i = 0; i < 6; i++) {
        moveServo(i, 90);
    }
    
    pinMode(JOYSTICK_BTN, INPUT_PULLUP);
    
    Serial.println("Robot Arm Ready!");
}

// ===== Main Loop =====
void loop() {
    // Check Bluetooth commands
    if (bluetooth.available()) {
        char command = bluetooth.read();
        handleCommand(command);
    }
    
    // Manual control with joystick
    if (!autoMode) {
        manualControl();
    }
    
    // Smooth movement
    for (int i = 0; i < 6; i++) {
        if (servoPositions[i] != targetPositions[i]) {
            if (servoPositions[i] < targetPositions[i]) {
                servoPositions[i]++;
            } else {
                servoPositions[i]--;
            }
            moveServo(i, servoPositions[i]);
        }
    }
    
    delay(15);
}

// ===== Manual Control =====
void manualControl() {
    int x = analogRead(JOYSTICK_X);
    int y = analogRead(JOYSTICK_Y);
    
    if (x < 300) targetPositions[0] = constrain(targetPositions[0] + 2, 0, 180);
    if (x > 700) targetPositions[0] = constrain(targetPositions[0] - 2, 0, 180);
    if (y < 300) targetPositions[1] = constrain(targetPositions[1] + 2, 0, 180);
    if (y > 700) targetPositions[1] = constrain(targetPositions[1] - 2, 0, 180);
    
    if (!digitalRead(JOYSTICK_BTN)) {
        targetPositions[2] = constrain(targetPositions[2] + 2, 0, 180);
    }
}

// ===== Handle Bluetooth Commands =====
void handleCommand(char cmd) {
    switch(cmd) {
        case '1':
            targetPositions[0] = constrain(targetPositions[0] + 10, 0, 180);
            break;
        case '2':
            targetPositions[0] = constrain(targetPositions[0] - 10, 0, 180);
            break;
        case '3':
            targetPositions[1] = constrain(targetPositions[1] + 10, 0, 180);
            break;
        case '4':
            targetPositions[1] = constrain(targetPositions[1] - 10, 0, 180);
            break;
        case '5':
            targetPositions[2] = constrain(targetPositions[2] + 10, 0, 180);
            break;
        case '6':
            targetPositions[2] = constrain(targetPositions[2] - 10, 0, 180);
            break;
        case 'G':
            performGrip();
            break;
        case 'R':
            resetPosition();
            break;
        case 'A':
            autoMode = !autoMode;
            if (autoMode) performWave();
            break;
    }
}

// ===== Move Servo =====
void moveServo(int index, int position) {
    switch(index) {
        case 0: servo1.write(position); break;
        case 1: servo2.write(position); break;
        case 2: servo3.write(position); break;
        case 3: servo4.write(position); break;
        case 4: servo5.write(position); break;
        case 5: servo6.write(position); break;
    }
}

// ===== Perform Grip =====
void performGrip() {
    targetPositions[5] = 45;
    delay(500);
    targetPositions[5] = 135;
}

// ===== Reset Position =====
void resetPosition() {
    for (int i = 0; i < 6; i++) {
        targetPositions[i] = 90;
    }
}

// ===== Wave Animation =====
void performWave() {
    for (int i = 0; i < 3; i++) {
        targetPositions[0] = 45;
        targetPositions[1] = 45;
        delay(300);
        targetPositions[0] = 135;
        targetPositions[1] = 135;
        delay(300);
    }
    resetPosition();
}
