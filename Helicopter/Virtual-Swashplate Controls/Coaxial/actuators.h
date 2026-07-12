#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>
#include <controlSystems.h>

// Pin definitions
#define motorPin PA1
#define servoPin PA6


extern void actuators_setup();
extern void actuators_write();

#endif
