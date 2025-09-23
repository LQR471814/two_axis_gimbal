#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This is a library that allows one to emit PWM with a hardware timer on the
 * ATmega328 and thus control a servo.
 *
 * The way we will implement PWM using the hardware timer on the
 * microcontroller is by:
 * - Using fast PWM mode to generate a HIGH before toggling to LOW once COM2A is
 *   reached.
 * - Using an interrupt on COM2B to reset the count on the timer at the correct
 *   threshold to ensure the period of the waveform is correct. An interrupt
 *   can introduce timing delays but I've done the calculations and found that
 *   the delay is mitigatible.
 *
 * Thus we will need to compute COM2A dynamically (as it depends on the angle),
 * while the rest of the hardware timer settings can be computed ahead of time
 * (ex. pre-scalar, COM2B threshold)
 *
 * We have a few constants specific to hardware (the microcontroller and the
 * servo involved):
 *
 * - K = CLOCK_SPEED  # the clock speed (Hz) of the ATmega328
 * - P = SERVO_PULSE_PERIOD  # the period of each pulse in the servo's PWM
 *   waveform (sec.)
 * - H_L = SERVO_DUTY_CYCLE_LOW_BOUND  # the lower bound for the duty cycle
 *   length (sec.)
 * - H_H = SERVO_DUTY_CYCLE_HIGH_BOUND  # the upper bound for the duty cycle
 *   length (sec.)
 * - C = TIMER_MAX_COUNT  # the maximum count the timer can reach before
 *   overflowing
 *
 * Note: The duty cycle of the PWM defines the angle of the servo. On the servo
 * we are given (1 ms = 0 deg., 2 ms = 180 deg.)
 *
 * We want to compute the minimum pre-scalar value (S) to set:
 *
 * v = K/S  # count/s
 * T = C/v  # pulse period (s)
 * T = CS/K
 *
 * Names:
 *
 * - S = TIMER_PRESCALAR
 * - T = TIMER_TOTAL_PERIOD
 *
 * In practice, S will most certainly be set such that T >= P, but we want to
 * compute the minimum value of S so that there is as little difference as
 * possible.
 *
 * To compute the threshold for COM2A where angle is `a`:
 *
 * h = a*(H_H - H_L) + H_L  # duty cycle (s)
 * M_a = h/T * C  # count at which COM2A should be matched, HIGH -> LOW
 * M_b = P/T * C  # count at which COM2B shoudl be matched, count cleared
 */

constexpr double CLOCK_SPEED = 16000000;
constexpr double SERVO_PULSE_PERIOD = 0.02;
constexpr double SERVO_DUTY_CYCLE_LOW_BOUND = 0.001;
constexpr double SERVO_DUTY_CYCLE_UP_BOUND = 0.002;
constexpr double TIMER_MAX_COUNT = 1 << 16;

struct timer_prescalar_values_t {
  uint8_t PRESCALAR_INDEX;
  double TOTAL_PERIOD;
};

constexpr double PRESCALAR_STEPS[5] = {1, 8, 64, 256, 1024};

int main(void) {
  uint8_t PRESCALAR_INDEX;
  double TIMER_TOTAL_PERIOD;

  // find the minimum prescalar such that the timer's maximum count is > than
  // the servo's PWM period
  for (uint8_t i = 0; i < 5; i++) {
    double prescalar = PRESCALAR_STEPS[i];
    double overflow_time = TIMER_MAX_COUNT * prescalar / CLOCK_SPEED;
    if (overflow_time < SERVO_PULSE_PERIOD) {
      continue;
    }
    PRESCALAR_INDEX = i;
    TIMER_TOTAL_PERIOD = overflow_time;
    break;
  }

  double SERVO_DUTY_CYCLE_LENGTH =
      SERVO_DUTY_CYCLE_UP_BOUND - SERVO_DUTY_CYCLE_LOW_BOUND;
  uint8_t PWM_OUTPUT_PIN = 3;
  uint8_t PRESCALAR_MASK = PRESCALAR_INDEX + 1;
  uint16_t TOP = SERVO_PULSE_PERIOD / TIMER_TOTAL_PERIOD * TIMER_MAX_COUNT - 1;

  printf("constexpr unsigned char PWM_OUTPUT_PIN = %d;\n", PWM_OUTPUT_PIN);
  printf("constexpr unsigned char PRESCALAR_MASK = 0b%08b;\n", PRESCALAR_MASK);
  printf("constexpr unsigned int TOP = %d; // period error: %lf%% \n", TOP,
         fabs(TOP * PRESCALAR_STEPS[PRESCALAR_INDEX] / CLOCK_SPEED -
              SERVO_PULSE_PERIOD) /
             SERVO_PULSE_PERIOD * 100);

  /**
   * x = angle
   * a = SERVO_DUTY_CYCLE_LENGTH
   * b = SERVO_DUTY_CYCLE_LOW_BOUND
   * c = TIMER_TOTAL_PERIOD
   * d = TIMER_MAX_COUNT
   *
   * (x / 180 * a + b) / c * d - 1
   * P = a / 180
   * Q = d / c
   * (x * P + b) * Q - 1
   * x * P * Q + b * Q - 1
   * m = P * Q
   * n = b * Q - 1
   * mx + n
   */

  double match_a_coeff =
      SERVO_DUTY_CYCLE_LENGTH / 180 * TIMER_MAX_COUNT / TIMER_TOTAL_PERIOD;
  printf("constexpr unsigned int MATCH_A_COEFF = %d;\n",
         (unsigned int)match_a_coeff);

  double match_a_offset =
      SERVO_DUTY_CYCLE_LOW_BOUND * TIMER_MAX_COUNT / TIMER_TOTAL_PERIOD - 1;
  printf("constexpr unsigned int MATCH_A_OFFSET = %d;\n",
         (unsigned int)match_a_offset);

  // compute expected error
  unsigned int scalar = 5;
  unsigned int n = 180 / scalar;
  double *errors = (double *)malloc(sizeof(double) * (180 / 5));
  for (int i = 0; i < n; i++) {
    double angle = scalar * i;
    double duty_cycle =
        angle / 180 * SERVO_DUTY_CYCLE_LENGTH + SERVO_DUTY_CYCLE_LOW_BOUND;
    uint16_t MATCH_A = duty_cycle / TIMER_TOTAL_PERIOD * TIMER_MAX_COUNT;
    errors[i] =
        fabs(MATCH_A / TIMER_MAX_COUNT * TIMER_TOTAL_PERIOD - duty_cycle) /
        duty_cycle;
  }
  for (int i = 0; i < n; i++) {
    double current = errors[i];
    double maxvalue = -1;
    int max = -1;
    for (int j = i + 1; j < n; j++) {
      double cmp = errors[j];
      if (cmp > maxvalue) {
        max = j;
        maxvalue = cmp;
      }
    }
    if (max < 0) {
      break;
    }
    errors[i] = maxvalue;
    errors[max] = current;
  }
  printf("// p95 error: %lf%%, p50 error: %lf%%\n",
         errors[n - n * 95 / 100] * 100, errors[n - n / 2] * 100);
  free(errors);

  return 0;
}
