#include <Servo.h>
#include <Wire.h>
#include "gyro.h"
#include "constants.h"

static Servo servo;

#ifdef DEBUG
#define PRINT_ERR(expr, str, exit) { uint8_t err = expr; if (err != 0) { Serial.print(str); Serial.print(" Err: "); Serial.print(err); Serial.print("\n"); exit; } }
#else
#define PRINT_ERR(expr, str, exit) expr;
#endif

static void reset_gyro() {
  PRINT_ERR(Gyro::setup(), "I2C setup failed!", return);
  Serial.println("I2C setup complete!");
}

static void setup(void) {
  Serial.begin(9600);
  Wire.begin();
  servo.attach(3);

  reset_gyro();
}

static void loop(void) {
  uint16_t gyro[3];
  uint16_t accel[3];

  PRINT_ERR(Gyro::read(gyro, accel), "Gyro read failed!", delay(1000); reset_gyro(); return);

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

  delay(50);
}

