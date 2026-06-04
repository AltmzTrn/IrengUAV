// Actuators.cpp
#include "Actuators.h"

const int motorPins[] = {PA6, PA7, PB0, PB1};

void actuators_setup() {
    for (int i = 0; i < 4; i++) {
        pinMode(motorPins[i], OUTPUT);
    }
}

void actuators_write() {
    for (int i = 0; i < 4; i++) {
        analogWrite(motorPins[i], to_actuator[i]); // values[i] range 0-255
    }
}