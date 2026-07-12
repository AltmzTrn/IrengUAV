#ifndef CRSF_HANDLER_H
#define CRSF_HANDLER_H

#include <Arduino.h>

namespace crsfProtocol {
    const uint8_t RC_CHANNEL_COUNT = 16;
}

extern int rcChannelValues[crsfProtocol::RC_CHANNEL_COUNT];
extern bool crsfFailsafe;

void crsf_setup();
void crsf_update();

#endif
