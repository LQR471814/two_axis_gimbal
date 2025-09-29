#include <Wire.h>
#include "gyro.h"
#include "constants.h"

static constexpr uint8_t MPU_ADDR = 0x68;

#ifdef DEBUG
#define TRY(expr, catch) { \
  uint8_t err = expr; \
  if (err != 0) { catch; } \
}
#else
#define TRY(expr, catch) expr;
#endif

namespace Gyro {
  static uint8_t write_reg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    TRY(Wire.write(reg) == 0, return 1)
    TRY(Wire.write(value) == 0, return 2)
    TRY(Wire.endTransmission(), return err + 10)
    return 0;
  }

  uint8_t setup(void) {
    // reg: PWR_MGMT_1
    // value: disables sleep mode, uses PLL with X axis gyro for clock
    TRY(write_reg(0x6B, 0x01), return err + 10)

    // reg: PWR_MGMT_2
    // value: all disabled
    TRY(write_reg(0x6C, 0x00), return err + 20)

    // reg: GYRO_CONFIG
    // use range +-250
    TRY(write_reg(0x1B, 0x00), return err + 30)

    // reg: ACCEL_CONFIG
    // use range +-2g
    TRY(write_reg(0x1C, 0x00), return err + 40)

    return 0;
  }

  static uint8_t read8(uint8_t* out) {
#ifdef DEBUG
    if (!Wire.available()) {
      return 1;
    }
#endif
    *out = Wire.read();
    return 0;
  }

  static uint8_t read16(int16_t* out) {
    uint8_t upper, lower;
    TRY(read8(&upper), return 1)
    TRY(read8(&lower), return 2)
    *out = lower | (upper << 8);
    return 0;
  }

  uint8_t read(int16_t gyro[3], int16_t accel[3]) {
    TRY(Wire.requestFrom(MPU_ADDR, 14, 0x3B, 1, true) != 14, return err)

    uint8_t err;

#define READ16(out, errprefix) err = read16(&(out)); \
    if (err != 0) { return errprefix + err; }

    READ16(accel[0], 10)
    READ16(accel[1], 20)
    READ16(accel[2], 30)

    // skip temp. data
    int16_t temp;
    err = read16(&temp);
#ifdef DEBUG
    if (err != 0) { return 40 + err; }
#endif

    READ16(gyro[0], 50)
    READ16(gyro[1], 60)
    READ16(gyro[2], 70)

#undef READ16

    return 0;
  }
}

