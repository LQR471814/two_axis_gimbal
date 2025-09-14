#include <array>
#include <tuple>

// the clock speed (Hz) of the ATmega328
constexpr double CLOCK_SPEED = 16_000_000;

// the period of each pulse in the servo's PWM waveform (sec.)
constexpr double SERVO_PULSE_PERIOD = 0.02;

// the duty cycle of the PWM defines the angle of the servo.
// ex. on the servo we are given:
// 1000 microsec duty cycle = 0 degrees
// 1500 microsec duty cycle = 90 degrees
// 2000 microsec duty cycle = 180 degrees

// the lower bound for the duty cycle length (sec.)
constexpr double SERVO_DUTY_CYCLE_LOW_BOUND = 0.001;

// the upper bound for the duty cycle length (sec.)
constexpr double SERVO_DUTY_CYCLE_UP_BOUND = 0.002;

// the PWM output pin
constexpr int PWM_OUTPUT_PIN = 9;

// the maximum count - 1 the timer can reach before overflowing
constexpr double TIMER_MAX_COUNT = 1 << 16;

// chooses:
// 1. smallest prescalar value for TIMER_OVERFLOW_TIME > SERVO_PULSE_PERIOD
// 2. the TIMER_OVERFLOW_TIME corresponding with that prescalar value
constexpr double compute_overflow_time(prescalar double) {
  return TIMER_MAX_COUNT * prescalar / CLOCK_SPEED;
}
constexpr std::tuple<int, double> choose_prescalar() {
  std::array<double> steps {1, 8, 32, 64, 128, 256, 1024};
  for (double prescalar : steps) {
    double overflow_time = compute_overflow_time(prescalar);
    if (overflow_time < SERVO_PULSE_PERIOD) {
      continue;
    }
    return std::make_tuple(prescalar, overflow_time);
  }
  static_assert(true, "No prescalar value can make the hardware timer overflow time exceed the servo pulse period");
}
constexpr double [TIMER_PRESCALAR, TIMER_OVERFLOW_TIME] = choose_prescalar();


Servo::Servo(unsigned int angle = 0) : angle{angle} {
  pinMode(PWM_OUTPUT_PIN, OUTPUT);

  // Timer configuration.
  //
  // Non-inverting compare output mode (OC1A output is Fast PWM non-inverting mode, OC1B output disabled)
  // COM1A1 (TCCR1A:7) = 1
  // COM1A0 (TCCR1A:6) = 0
  // COM1B1 (TCCR1A:5) = 0
  // COM1B0 (TCCR1A:4) = 0
  //
  // Fast PWM mode with TOP =
  // WGM21 (TCCR1A:1) = 1
  // WGM20 (TCCR1A:0) = 1
  // WGM22 (TCCR1B:3) = 1
  //
  // Pre-scalar (clk/128)
  // CS22 (TCCR1B:2) = 1
  // CS21 (TCCR1B:1) = 0
  // CS20 (TCCR1B:0) = 1
  //
  // All other bits are reserved or 0 in this config.
  TCCR1A = 0b1000_0011;
  TCCR1B = 0b0000_1101;
}

void Servo::write(unsigned int angle) {
  this.angle = angle / 180.0;
}

unsigned int Servo::getAngle() {
  return (unsigned int) (this.angle * 180);
}

