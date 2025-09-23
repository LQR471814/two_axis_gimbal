#ifndef SERVO_H
#define SERVO_H

class Servo {
private:
  // angle is a value from [0, 1]
  unsigned int angle;

public:
  Servo(unsigned int angle = 0);
  void write(unsigned int angle);
  void setup();
};

#endif
