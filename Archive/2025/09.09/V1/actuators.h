#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>
#include <controlSystems.h>

// Pin definitions
#define motorPin PA1
#define servoFPin PA6
#define servoRPin PA7
#define servoLPin PB0

extern void actuators_setup();
extern void actuators_write();

#endif
