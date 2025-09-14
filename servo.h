#ifndef SERVO_H
#define SERVO_H

class Servo {
private:
  // angle is a value from [0, 1]
  double angle;

public:
  Servo(unsigned int angle = 0);
  void write(unsigned int angle);
  unsigned int getAngle();
};

#endif
