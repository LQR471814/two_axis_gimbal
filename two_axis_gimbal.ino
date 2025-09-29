#include <Servo.h>
#include <Wire.h>
#include "gyro.h"
#include "constants.h"
#include <math.h>

static Servo servo1;
static Servo servo2;

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

uint16_t last_micros = 0;

constexpr int16_t SERVO1_UP_ANGLE = 180;
constexpr int16_t SERVO2_UP_ANGLE = 90;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setWireTimeout();

  servo1.attach(3);
  servo2.attach(6);
  reset_gyro();

  last_micros = micros();

  Serial.println("Make sure the device is flat!");
  servo1.write(90);
  servo2.write(90);
  delay(1000);
}

float angles[3] { 0, 0, 0 };
float gyrof[3] { 0, 0, 0 };
float accelf[3] { 0, 0, 0 };

void loop() {
  int16_t gyro[3];
  int16_t accel[3];

  PRINT_ERR(Gyro::read(gyro, accel), "Gyro read failed!", delay(1000); reset_gyro(); return);

  uint16_t now = micros();
  float dt = (now - last_micros) / 1000000.0;
  last_micros = now;

  // 2^15 units / 250 degrees = 131.072  unit/deg
  gyrof[0] = gyro[0] / 131.072;
  gyrof[1] = gyro[1] / 131.072;
  // 2^15 units / 2 g = 16384 unit/g
  accelf[0] = accel[0] / 16384.0;
  accelf[1] = accel[1] / 16384.0;
  accelf[2] = accel[2] / 16384.0;

  // atan(X/Z) yields pitch (rotation about Y)
  float measured_pitch = -atan2(accelf[0], accelf[2]);

  // atan(Y/Z) yields roll (rotation about X)
  float measured_roll = atan2(accelf[1], accelf[2]);

  angles[0] = next_angle(dt, angles[0], gyrof[0], measured_roll * 180 / PI);
  angles[1] = next_angle(dt, angles[1], gyrof[1], measured_pitch * 180 / PI);

  // servo 1 should be at 90 degrees at 0 observed degrees.
  // servo 2 should be at 90 degrees at 0 observed degrees.
  int16_t a1 = ((int16_t) angles[0]) + 90;
  int16_t a2 = ((int16_t) angles[1]) + 90;

  if (a1 > 180) {
    a1 = 180;
  } else if (a1 < 0) {
    a1 = 0;
  }
  if (a2 > 180) {
    a2 = 180;
  } else if (a2 < 0) {
    a2 = 0;
  }

  servo1.write(a1);
  servo2.write(a2);

  Serial.print(a1);
  Serial.print(" ");
  Serial.print(a2);
  Serial.println();

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

  // Serial.print(",");
  // Serial.print(angles[1]);
  // Serial.print(",");
  // Serial.print(angles[2]);
  // Serial.print(",");
  // Serial.print(gyrof[0]);
  // Serial.print(",");
  // Serial.print(gyrof[1]);
  // Serial.print(",");
  // Serial.print(gyrof[2]);
  // Serial.print(",");
  // Serial.print(accelf[0]);
  // Serial.print(",");
  // Serial.print(accelf[1]);
  // Serial.print(",");
  // Serial.print(accelf[2]);
  // Serial.print(",");
  // Serial.print("\n");

  delay(20);
}

