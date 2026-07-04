#include <Joystick_ESP32S2.h>
Joystick_ Joystick;

const int PIN_STEERING = 4;
const int PIN_THROTTLE = 10;
const int PIN_BRAKE = 15;
const int PIN_HANDBRAKE = 16;
const int BTN_PINS[5] = {17, 18, 12, 13, 14};

int lastSteer = 0, lastThrottle = 0;

void setup(){
  USB.PID(0x8211);
  USB.VID(0x303b);
  USB.begin();

  Joystick.setXAxisRange(-127, 127);
  Joystick.setYAxisRange(0, 255);
  Joystick.begin(false);

  pinMode(PIN_BRAKE, INPUT_PULLUP);
  pinMode(PIN_HANDBRAKE, INPUT_PULLUP);
  for(int i = 0; i < 5; i++){
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }
}

void loop(){
  // دركسيون
  int steerRaw = analogRead(PIN_STEERING);
  int steer = map(steerRaw, 0, 4095, -127, 127);
  steer = constrain(steer, -127, 127);
  if(abs(steer - lastSteer) > 1){
    Joystick.setXAxis(steer);
    lastSteer = steer;
  }

  // بنزين - 2140 سايبة = 127 نص، 4095 دايسة = 255
  int thrRaw = analogRead(PIN_THROTTLE);

  thrRaw = map(thrRaw, 2140, 2900, 127, 255);
  thrRaw = constrain(thrRaw, 127, 255);

  if(abs(thrRaw - lastThrottle) > 1){
    Joystick.setYAxis(thrRaw);
    lastThrottle = thrRaw;
  }

  // أزرار
  Joystick.setButton(0,!digitalRead(PIN_BRAKE));
  Joystick.setButton(1,!digitalRead(PIN_HANDBRAKE));
  for(int i = 0; i < 5; i++){
    Joystick.setButton(i + 2,!digitalRead(BTN_PINS[i]));
  }

  Joystick.sendState();
  delay(5);
}
