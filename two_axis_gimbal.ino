#include "servo.h"

Servo servo;

void setup(void) {
  Serial.begin(9600);

  // defaults should be:
  // - phase correct PWM 8-bit
  // - prescalar 64
  // - TOP = 255
  // - OCR1A = 0
  // - OC1A disconnected
  //
  // duty cycle time (s): 2 * (TOP - OCR1A) * 64 / (16*10^6)
  // total cycle time (s): 2 * OCR1A * 64 / (16*10^6)
  //
  // set COM1A0 = 1 (toggle mode)
  // OCR1A = value from [0, 130]

  Serial.print(TCCR1A);
  Serial.print(" ");
  Serial.print(TCCR1B);
  Serial.print(" ");
  Serial.print(TCCR1C);
  Serial.print("\n");
  servo.setup();
}

void loop(void) {
  // Sweep from 0° to 180° and back
  for (unsigned int pos = 0; pos <= 180; pos++) {
    servo.write(pos);
    delay(15);
  }
  for (unsigned int pos = 180; pos >= 0; pos--) {
    servo.write(pos);
    delay(15);
  }
}

