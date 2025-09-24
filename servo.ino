#include "servo_constants.h"
#include "servo.h"

Servo::Servo(unsigned int angle = 0) : angle{angle} {}

void Servo::setup() {
  cli();

  // stop timer1
  TCNT1 = 0;
  TCCR1A = 0;
  TCCR1B = 0;

  /*
   * Timer configuration.
   *
   * Compare output mode (non-inverting mode)
   * COM1A1 (TCCR1A:7) = 1
   * COM1A0 (TCCR1A:6) = 0
   * COM1B1 (TCCR1A:5) = 0
   * COM1B0 (TCCR1A:4) = 0
   *
   * Fast PWM mode
   * WGM10 (TCCR1A:0) = 0
   * WGM11 (TCCR1A:1) = 1
   * WGM12 (TCCR1B:3) = 1
   * WGM13 (TCCR1B:4) = 1
   *
   * Pre-scalar (001 = clk/8)
   * CS22 (TCCR1B:2) = highest bit
   * CS21 (TCCR1B:1)
   * CS20 (TCCR1B:0) = lowest bit
   *
   * Interrupt mask register
   * OCIE1B = 1
   *
   * All other bits are reserved or 0 in this config.
   */

  // phase correct PWM
  //
  // to_sec(count) = count * pre_scalar / (16 * 10^6)
  // TOP = ICR1
  // duty_cycle = 2 * to_sec(TOP - ICR1)
  // full_period = duty_cycle + 2 * to_sec(ICR1)

  // prescalar = 8
  // phase correct PWM
  // toggle on compare match

  TCCR1A = (1 << WGM11) | (1 << COM1A0);
  TCCR1B = (1 << WGM13) | (1 << CS11);
  ICR1 = 20000;

  this->write(angle);

  // // enable global interrupt bit
  // TIMSK1 = (1 << TOIE1) | (1 << OCIE1A) | (1 << OCIE1B);

  pinMode(PWM_OUTPUT_PIN, OUTPUT);

  sei();
}

// #if defined(TIM1_COMPB_vect)
// ISR(TIM1_COMPB_vect)
// #else
// ISR(TIMER1_COMPB_vect)
// #endif
// {
//   TCNT1 = 0;
// }

void Servo::write(unsigned int angle) {
  if (angle >= 1000) {
    angle = 1000;
  }
  this->angle = angle;

  uint16_t MATCH_A = 18000 + angle;

  // this is a 16-bit write, so it is technically 2 instructions which
  // technically opens the door for race conditions with interrupts, but in our
  // case that probably won't ever happen
  // OCR1A = MATCH_A;
  OCR1A = MATCH_A;
}

