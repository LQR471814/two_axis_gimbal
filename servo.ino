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

  TCCR1A = 0b10000010;
  TCCR1B = 0b00011000 | PRESCALAR_MASK;
  ICR1   = TOP;

  this->write(angle);

  // enable global interrupt bit
  TIMSK1 = (1 << TOIE1) | (1 << OCIE1A) | (1 << OCIE1B);

  pinMode(PWM_OUTPUT_PIN, OUTPUT);

  sei();
}

#if defined(TIM1_COMPB_vect)
ISR(TIM1_COMPB_vect)
#else
ISR(TIMER1_COMPB_vect)
#endif
{
  TCNT1 = 0;
}

void Servo::write(unsigned int angle) {
  if (angle < 0) {
    angle = 0;
  }
  if (angle >= 180) {
    angle = 180;
  }
  this->angle = angle;

  uint16_t MATCH_A = MATCH_A_COEFF * angle + MATCH_A_OFFSET;

  // this is a 16-bit write, so it is technically 2 instructions which
  // technically opens the door for race conditions with interrupts, but in our
  // case that probably won't ever happen
  // OCR1A = MATCH_A;
  OCR1A = MATCH_A;
}

