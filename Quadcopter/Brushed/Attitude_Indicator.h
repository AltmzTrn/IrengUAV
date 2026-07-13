#ifndef ATTITUDE_INDICATOR_H
#define ATTITUDE_INDICATOR_H

#include <Arduino.h>

extern int Attitude[3];
extern float gyroDegS[3];

void IMU_init();
void IMU_update();

#endif