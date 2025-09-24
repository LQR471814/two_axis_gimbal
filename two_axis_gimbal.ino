// #include "servo.h"
//
// Servo servo;
//
// void setup(void) {
//   Serial.begin(9600);
//
//   // defaults should be:
//   // - phase correct PWM 8-bit
//   // - prescalar 64
//   // - TOP = 255
//   // - OCR1A = 0
//   // - OC1A disconnected
//   //
//   // duty cycle time (s): 2 * (TOP - OCR1A) * 64 / (16*10^6)
//   // total cycle time (s): 2 * OCR1A * 64 / (16*10^6)
//   //
//   // set COM1A0 = 1 (toggle mode)
//   // OCR1A = value from [0, 130]
//
//   servo.setup();
//
//   // before: 0000_0001
//   // after: 0100_0010
//   Serial.print(TCCR1A);
//   Serial.print(" ");
//   // before: 0000_0011
//   // after: 0000_1110
//   Serial.print(TCCR1B);
//   Serial.print(" ");
//   Serial.print(TCCR1C);
//   Serial.print("\n");
// }
//
// void loop(void) {
//   // Sweep from 0° to 180° and back
//   for (unsigned int pos = 0; pos <= 180; pos++) {
//     servo.write(pos);
//     delay(15);
//   }
//   for (unsigned int pos = 180; pos >= 0; pos--) {
//     servo.write(pos);
//     delay(15);
//   }
// }

// Timer1 -> 50 Hz servo PWM on D9 (OC1A)
static inline void servoWriteMicroseconds(uint16_t us) {
  // prescaler = 8 -> timer tick = F_CPU/8
  const uint16_t ticks_per_us = (F_CPU / 8) / 1000000UL; // 2 at 16MHz, 1 at 8MHz
  OCR1A = us * ticks_per_us;  // 1000–2000 us typical
}

void setup() {
  pinMode(9, OUTPUT);           // D9 = OC1A

  // Reset Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // Fast PWM, TOP = ICR1  (WGM13:0 = 14 -> 1110b)
  TCCR1A = (1 << WGM11);                   // WGM11=1, WGM10=0
  TCCR1B = (1 << WGM13) | (1 << WGM12);    // WGM13=1, WGM12=1

  // Non-inverting PWM on OC1A (D9)
  TCCR1A |= (1 << COM1A1);   // COM1A1:0 = 2 (non-inverting)

  // Prescaler = 8 (enable the clock last to avoid glitches)
  TCCR1B |= (1 << CS11);

  // Set TOP for 50 Hz:
  // TOP = Fclk/(prescale*freq) - 1 = F_CPU/(8*50) - 1
  ICR1 = (F_CPU / 8 / 50) - 1;  // 39,999 at 16MHz; 19,999 at 8MHz

  // Center the servo (≈1.5 ms)
  servoWriteMicroseconds(1500);
}

void loop() {
  // Example sweep
  for (uint16_t us = 1000; us <= 2000; us += 10) {
    servoWriteMicroseconds(us);
    delay(15);
  }
  for (uint16_t us = 2000; us >= 1000; us -= 10) {
    servoWriteMicroseconds(us);
    delay(15);
  }
}
