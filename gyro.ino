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

static uint8_t write_reg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    TRY(Wire.write(reg) == 0, return 1);
    TRY(Wire.write(value) == 0, return 2);
    TRY(Wire.endTransmission(), return err + 10);
    return 0;
}

namespace Gyro {
  uint8_t setup(void) {
    // reg: PWR_MGMT_1
    // value: trigger DEVICE_RESET
    TRY(write_reg(0x6B, 1 << 7), return err);
    delay(100);

    // reg: PWR_MGMT_1
    // value: disables sleep mode, uses PLL with X axis gyro for clock
    TRY(write_reg(0x6B, 0x01), return err + 10);

    // reg: PWR_MGMT_2
    // value: all disabled
    TRY(write_reg(0x6C, 0x00), return err + 20);

    // reg: GYRO_CONFIG
    // use range +-250
    TRY(write_reg(0x1B, 0x00), return err + 30);

    // reg: ACCEL_CONFIG
    // use range +-8g
    TRY(write_reg(0x1C, 0x02), return err + 40);

    return 0;
  }

  uint8_t read(int (&gyro)[3], int (&accel)[3]) {
    // request registers from ACCEL_REG -> GYRO_REG from device
    // this guarantees that all register data is from the same sample
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

