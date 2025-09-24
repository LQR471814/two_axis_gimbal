#include <Servo.h>
#include <Wire.h>
#include "gyro.h"

static Servo servo;

#define PRINT_ERR(expr, str, exit) { uint8_t err = expr; if (err != 0) { Serial.print(str); Serial.print(" Err: "); Serial.print(err); Serial.print("\n"); exit; } }

static void setup(void) {
  Serial.begin(9600);
  Wire.begin();
  servo.attach(3);

  PRINT_ERR(Gyro::setup(), "I2C setup failed!", return);
  Serial.println("I2C setup complete!");
}

// observations:
//   o1: error code (1) - write() when buffer full
//   o1: error code (2) - received nack on endTransmission()
//
// what works:
//


// what works:
//   k1: no errors
// problem: gyroscope values are not changing
//   p1: gyroscope not on

static void loop(void) {
  int gyro[3];
  int accel[3];

  PRINT_ERR(Gyro::read(gyro, accel), "Gyro read failed!", return);

  Serial.print("GYRO: ");
  Serial.print(gyro[0]);
  Serial.print(" ");
  Serial.print(gyro[1]);
  Serial.print(" ");
  Serial.print(gyro[2]);
  Serial.print(" | ACCEL: ");
  Serial.print(accel[0]);
  Serial.print(" ");
  Serial.print(accel[1]);
  Serial.print(" ");
  Serial.print(accel[2]);
  Serial.print("\n");

  delay(1000);
}

