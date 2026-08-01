#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>
#include <controlSystems.h>

// Pin definitions
#define motorPin PA1
#define servoPin PB0
#define encoderPin PA0


extern void actuators_setup();
extern void actuators_write();
extern void encoder_read();

unsigned long 


#endif
