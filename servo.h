#ifndef SERVO_H
#define SERVO_H

class Servo {
private:
  // angle is a value from [0, 1]
  double angle;

public:
  Servo(double angle = 0);
  void write(double angle);
  void setup(void);
  double getAngle(void);
};

#endif
