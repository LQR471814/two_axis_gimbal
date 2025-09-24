#include "gyro.h"

static constexpr int MPU_ADDR = 0x68;
static constexpr int ACCEL_REG = 59;
static constexpr int GYRO_REG = 67;

#define TRY(expr, catch) { uint8_t err = expr; if (err != 0) { catch; } }

/**
 * Notes on the Wire.h library.
 *
 * beginTransmission() -> doesn't actually send anything, just sets some
 * configuration variables.
 *
 * write() -> writes a value to a buffer and updates some variables, doesn't
 * send anything either. returns 0 if buffer is full
 *
 * endTransmission() -> actually executes the send, this includes the address
 * and the things inside the buffer
 */

namespace Gyro {
  uint8_t setup(void) {
    // internals:
    // beginTransmission just sets a bunch of variables, it does not actually
    // call any twi functions to transmit anything
    Wire.beginTransmission(MPU_ADDR);

    // PWR_MGMT_1 register
    TRY(Wire.write(0x6B) != 0, return 1);
    TRY(Wire.endTransmission(false), return err + 10);

    // disables sleep mode, uses PLL with X axis gyro for clock
    TRY(Wire.write(0b00000001) != 0, return 2);
    TRY(Wire.endTransmission(true), return err + 20);

    return 0;
  }

  uint8_t read(int (&gyro)[3], int (&accel)[3]) {
    // request internal register ACCEL_REG from device
    TRY(Wire.requestFrom(MPU_ADDR, 6, ACCEL_REG, 2, true), return err);

    char i = 0;
    while (Wire.available() && i < 3) {
      char upper = Wire.read();
      if (!Wire.available()) {
        break;
      }
      char lower = Wire.read();
      accel[i] = lower | (upper << 8);
      i++;
    }

    // request internal register GYRO_REG from device
    TRY(Wire.requestFrom(MPU_ADDR, 6, GYRO_REG, 2, true), return err);

    i = 0;
    while (Wire.available() && i < 3) {
      char upper = Wire.read();
      if (!Wire.available()) {
        break;
      }
      char lower = Wire.read();
      gyro[i] = lower | (upper << 8);
      i++;
    }

    return 0;
  }
}

