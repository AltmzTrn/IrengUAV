#include "actuators.h"
#include "controlSystems.h"

void writeServoMicros(uint8_t pin, uint16_t us) {
  analogWriteResolution(16);  // 0 - 65535

  const uint32_t pwmFreq = 50;           // 50 Hz for servos and ESC
  const uint32_t timerPeriod = 1000000 / pwmFreq; // Period in microseconds (20ms)

  // Convert desired pulse width to duty
  uint32_t duty = (us * 65535UL) / timerPeriod;
  analogWrite(pin, duty);
}

void actuators_setup() {
  pinMode(motorPin, OUTPUT);
  pinMode(servoPin, OUTPUT);

  analogWriteFrequency(50);
}


void actuators_write() {
  // Write directly as microsecond PWM signals
  writeServoMicros(motorPin, to_actuator[0]);
  writeServoMicros(servoPin, to_actuator[1]);
}

void encoder_read() {
  unsigned long pulseDuration = pulseIn(encoderPin, HIGH, 5000);
  if (pulseDuration>0) {
    int angle = map(pulseDuration, 12,4084,0,4095);
    angle = constrain(angle, 0, 4095);
    float degrees = (angle/4095)*360.0;
    
  }
}


