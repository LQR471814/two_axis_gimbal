#ifndef GYRO_H
#define GYRO_H

namespace Gyro {
	uint8_t setup(void);
	uint8_t read(int (&gyro)[3], int (&accel)[3]);
};

#endif
