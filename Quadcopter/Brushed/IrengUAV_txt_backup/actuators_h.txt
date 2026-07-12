#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>
#include <controlSystems.h>

// Pin definitions
#define motor_R_F PA6
#define motor_L_F PA7
#define motor_R_R PB0
#define motor_L_R PB1

extern void actuators_setup();
extern void actuators_write();

#endif
