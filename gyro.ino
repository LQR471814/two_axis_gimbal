#include "gyro.h"
#include "constants.h"

static constexpr int MPU_ADDR = 0x68;
static constexpr int ACCEL_REG = 0x3B;
static constexpr int GYRO_REG = 0x43;

#ifdef DEBUG
#define TRY(expr, catch) { uint8_t err = expr; if (err != 0) { catch; } }
#else
#define TRY(expr, catch) expr;
#endif

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

static uint8_t write_reg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    TRY(Wire.write(reg) == 0, return 1);
    TRY(Wire.endTransmission(false), return err + 10);
    TRY(Wire.write(value) == 0, return 2);
    TRY(Wire.endTransmission(true), return err + 20);
    return 0;
}

namespace Gyro {
  uint8_t setup(void) {
    // reg: 0x6B
    // value: DEVICE_RESET
    TRY(write_reg(0x6B, 0b10000000), return err);

    // wait 100ms for reset to complete
    delay(100);

    // reg: PWR_MGMT_1
    // value: disables sleep mode, uses PLL with X axis gyro for clock
    TRY(write_reg(0x6B, 0b00000001), return err + 20);

    // reg: PWR_MGMT_2
    // value: all disabled
    TRY(write_reg(0x6C, 0), return err + 40);

    // reg: GYRO_CONFIG
    // enable self test and use FSR +-250
    TRY(write_reg(0x1B, 0b11100000), return err + 60);

    // reg: ACCEL_CONFIG
    // enable self test and use FSR +-8g
    TRY(write_reg(0x1C, 0b11100010), return err + 80);

    return 0;
  }

  uint8_t read(int (&gyro)[3], int (&accel)[3]) {
    // request registers from ACCEL_REG -> GYRO_REG from device
    TRY(Wire.requestFrom(MPU_ADDR, 14, ACCEL_REG, 1, true) != 14, return err);

    char i = 0;
    while (i < 3) {
      if (!Wire.available()) {
        return 120;
      }
      char upper = Wire.read();
      if (!Wire.available()) {
        return 120;
      }
      char lower = Wire.read();
      accel[i] = lower | (upper << 8);
      i++;
    }

    // skip temp. data
    if (!Wire.available()) {
      return 130;
    }
    Wire.read();
    if (!Wire.available()) {
      return 130;
    }
    Wire.read();

    i = 0;
    while (i < 3) {
      if (!Wire.available()) {
        return 140;
      }
      char upper = Wire.read();
      if (!Wire.available()) {
        return 140;
      }
      char lower = Wire.read();
      gyro[i] = lower | (upper << 8);
      i++;
    }

    return 0;
  }
}

