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

// observations:
//   o1.1: no bytes are available() for reading, however reading does not fail
//   o1.2: read can fail silently in cases where a timeout is exceeded
//   o1.3: timeout should be disabled by default
//   o1.4: read can read no data in cases where no data is indeed read
//
//   o2.1: green light on gyroscope when pressed against pins
//   o2.2: green light does not light when board is obviously rotated around
//
//   o3.1:   error code (2) - received NACK on endTransmission()
//   o3.2:   recv NACK status is read from TWSR
//   o3.3:   CASE 1: incorrect address
//   o3.3.1: NACK is sent as a result of both addresses, NOT case 1
//   o3.4:   CASE 2: sleep mode
//   o3.4.1: very unlikely sleep mode will prevent I2C from working
//   o3.5:   CASE 3: incorrect I2C impl. somewhere
//   o3.5.1: CASE 3: very unlikely, implementation is widely used, board is
//           also highly rated
//   o3.6:   CASE 4: VLOGIC pin sets logic levels, logic levels incorrect for some reason
//   o3.6.1: VLOGIC is likely tied to VCC, so it is likely set correctly
//   o3.7:   CASE 5: power supply is incorrect
//   o3:     CONCLUSION: suboptimal physical contact was the root of the issue.
//           (Make sure you pull the chip upwards so that the right side of the
//           holes are touching the right side of the pins)
//
//   o4.1: error code (6) on requestFrom()
//   o4.2: CONCLUSION: requestFrom() returns number of bytes read, not error
//         code
//
//   o5.1: accel and gyro values not changing despite present acceleration and
//         rotation
//   o5.2: not the result of any settings in register PWR_MGMT_2
//   o5.3: not the result of the choice of clock source
//   o5.4: not the result of sending 8-bit addr. instead of 16-bit addr.
//         (though this did make the numbers correct)
//   o5.5: not the result of slow sampling
//   o5.6: *seemingly* sensible values come out from remove
//         endTransmission(false) after writing the address
//   o5: CONCLUSION: write to register was incorrect, write should not have
//       premature endTransmission in contrast to read
//
//   o6.1: after suboptimal physical contact, accel and gyro values are back to
//         not changing.

// conjectures:
//   c1: by observations in o1.2-4, no data is being read
//
//   c2: based on o2.1 and o2.2, the green light likely indicates that stable
//       contact with MCU and power is possible
//
//   c3: NACK on write to poweron is preventing data from being read
//
//   c6: this is likely due to the fact that the suboptimal physical contact
//       will cause the device to reset. therefore, setup should be rerun after
//       coming across a read fail error


static void loop(void) {
  int gyro[3];
  int accel[3];

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

