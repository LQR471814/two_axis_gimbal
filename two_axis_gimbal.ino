#include "servo.h"

Servo servo;

void setup(void) {
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

