#ifndef CONTROL_SYSTEMS_H
#define CONTROL_SYSTEMS_H

#include <Arduino.h>
#include <CRSFHandler.h>
#include <Attitude_Indicator.h>
#include <actuators.h>

// Define actuator pins
#define motorPin     PA1
#define servoFPin    PA6
#define servoRPin    PA7
#define servoLPin    PB0

extern uint16_t to_actuator[6];
extern uint16_t control_value[7];

void controlSystems_setup();
void controlSystems_update();
extern void writeServoMicros(uint8_t pin, uint16_t us);

#endif
