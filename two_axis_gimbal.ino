// #include "servo.h"

// Servo servo;

void setup(void) {
  // Serial.begin(9600);
  // Serial.write("Hello!\n");
  pinMode(LED_BUILTIN, OUTPUT);
  // servo.setup();
}

// int pos = 0;

void loop(void) {
  // Serial.write("Hello!\n");
  // delay(100);
  // for (pos = 0; pos <= 180; pos += 1) {
  //   servo.write((double) pos);
  //   delay(15);
  // }
  // for (pos = 180; pos >= 0; pos -= 1) {
  //   servo.write((double) pos);
  //   delay(15);
  // }
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on
  delay(1000);                  // wait for 1 second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off
  delay(1000);                  // wait for 1 second
}

