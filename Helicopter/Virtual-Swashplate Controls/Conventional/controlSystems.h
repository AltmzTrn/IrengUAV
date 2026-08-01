#ifndef CONTROL_SYSTEMS_H
#define CONTROL_SYSTEMS_H

#include <Arduino.h>
#include <CRSFHandler.h>
#include <Attitude_Indicator.h>
#include <actuators.h>

extern uint16_t to_actuator[6];

extern int16_t desVal[3];
extern int16_t actVal[3];

extern uint16_t armChannel;

void controlSystems_setup();
void controlSystems_update();
extern void writeServoMicros(uint8_t pin, uint16_t us);



#endif
