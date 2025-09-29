#include <Servo.h>
#include <Wire.h>
#include "gyro.h"
#include "constants.h"
#include <math.h>

static Servo servo;

#ifdef DEBUG
#define PRINT_ERR(expr, str, exit) { \
  uint8_t err = expr; \
  if (err != 0) { \
    Serial.print(str); \
    Serial.print(" Err: "); \
    Serial.print(err); \
    Serial.print("\n"); \
    exit; \
  } \
}
#else
#define PRINT_ERR(expr, str, exit) expr;
#endif

static void reset_gyro() {
  PRINT_ERR(Gyro::setup(), "I2C setup failed!", return);
  Serial.println("I2C setup complete!");
}

static inline float next_angle(float dt, float angle_prev, float angle_change, float angle_measured) {
  return 0.98 * (angle_prev + angle_change * dt) + 0.02 * angle_measured;
}

unsigned long last = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setWireTimeout();

  servo.attach(3);
  reset_gyro();

  last = micros();
}

float angles[3] { 0, 0, 0 };
float gyrof[3] { 0, 0, 0 };
float accelf[3] { 0, 0, 0 };

void loop() {
  int16_t gyro[3];
  int16_t accel[3];

  PRINT_ERR(Gyro::read(gyro, accel), "Gyro read failed!", delay(1000); reset_gyro(); return);
  float dt = (micros() - last) / 1000000.0;

  // 2^15 units / 250 degrees = 131.072  unit/deg
  gyrof[0] = gyro[0] / 131.072;
  gyrof[1] = gyro[1] / 131.072;
  gyrof[2] = gyro[2] / 131.072;
  // 2^15 units / 2 g = 16384 unit/g
  accelf[0] = accel[0] / 16384.0;
  accelf[1] = accel[1] / 16384.0;
  accelf[2] = accel[2] / 16384.0;

  // atan(X/Z) yields pitch (rotation about Y)
  float measured_pitch = atan2(accelf[0], accelf[2]);

  // atan(Y/Z) yields roll (rotation about X)
  float measured_roll = atan2(accelf[1], accelf[2]);

  angles[0] = next_angle(dt, angles[0], gyrof[0], measured_roll);
  angles[1] = next_angle(dt, angles[1], gyrof[1], measured_pitch);
  // yaw just ends up drifting
  angles[2] = angles[2] + gyrof[2] * dt;

  // Serial.print("| GYRO: ");
  // Serial.print(gyro[0]);
  // Serial.print(" ");
  // Serial.print(gyro[1]);
  // Serial.print(" ");
  // Serial.print(gyro[2]);
  // Serial.print(" | ACCEL: ");
  // Serial.print(accel[0]);
  // Serial.print(" ");
  // Serial.print(accel[1]);
  // Serial.print(" ");
  // Serial.print(accel[2]);
  // Serial.print("\n");

  Serial.print(angles[0]);
  Serial.print(",");
  Serial.print(angles[1]);
  Serial.print(",");
  Serial.print(angles[2]);
  Serial.print(",");
  Serial.print(gyrof[0]);
  Serial.print(",");
  Serial.print(gyrof[1]);
  Serial.print(",");
  Serial.print(gyrof[2]);
  Serial.print(",");
  Serial.print(accelf[0]);
  Serial.print(",");
  Serial.print(accelf[1]);
  Serial.print(",");
  Serial.print(accelf[2]);
  Serial.print(",");
  Serial.print("\n");

  delay(20);
}

