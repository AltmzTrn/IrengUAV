#include "actuators.h"
#include "controlSystems.h"

void writeServoMicros(uint8_t pin, uint16_t us) {
  const uint32_t pwmFreq = 490;   // Fast PWM refresh rate this ESC wants, not standard 50Hz servo rate
  const uint32_t timerPeriod = 1000000 / pwmFreq; // Period in microseconds

  // Convert desired pulse width to duty
  uint32_t duty = (us * 65535UL) / timerPeriod;
  analogWrite(pin, duty);
}

void actuators_setup() {
  pinMode(MainRotorPin, OUTPUT);
  pinMode(TailRotorPin, OUTPUT);

  analogWriteResolution(16);  // 0 - 65535, shared by both pins
  analogWriteFrequency(490);
}


void actuators_write() {
  // main rotor still wants a real 1000-2000us pulse, just refreshed faster
  writeServoMicros(MainRotorPin, to_actuator[0]);
  analogWrite(TailRotorPin, to_actuator[1]);
}

void encoder_read() {
  unsigned long pulseDuration = pulseIn(encoderPin, HIGH, 5000);
  if (pulseDuration>0) {
    int angle = map(pulseDuration, 12,4084,0,4095);
    angle = constrain(angle, 0, 4095);
    float degrees = (angle/4095.0)*360.0;
    
  }
}