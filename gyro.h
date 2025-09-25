#ifndef GYRO_H
#define GYRO_H

namespace Gyro {
	uint8_t setup(void);
	uint8_t read(uint16_t gyro[3], uint16_t accel[3]);
};

#endif
