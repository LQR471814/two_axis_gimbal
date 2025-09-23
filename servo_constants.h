constexpr unsigned char PWM_OUTPUT_PIN = 3;
constexpr unsigned char PRESCALAR_MASK = 0b00000010;
constexpr unsigned int TOP = 39999; // period error: 0.002500% 
constexpr unsigned int MATCH_A_COEFF = 11;
constexpr unsigned int MATCH_A_OFFSET = 1999;
// p95 error: 0.030769%, p50 error: 0.015094%
