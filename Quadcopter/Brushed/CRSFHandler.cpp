#include "CRSFHandler.h"

HardwareSerial crsfSerial(PB11, PB10);

int rcChannelValues[crsfProtocol::RC_CHANNEL_COUNT] = {0};
bool crsfFailsafe = true;

#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8
#define CRSF_FRAME_RC_CHANNELS_PACKED  0x16
#define CRSF_FRAME_LENGTH              24 // 1 (addr) + 1 (len) + 1 (type) + 22 (payload)

static uint8_t buffer[CRSF_FRAME_LENGTH];
static uint8_t bufferIndex = 0;

void crsf_setup() {
    crsfSerial.begin(420000, SERIAL_8N1);
}

void crsf_update() {
    while (crsfSerial.available()) {
        uint8_t byteIn = crsfSerial.read();

        buffer[bufferIndex++] = byteIn;

        // Check if we have enough to consider a valid frame
        if (bufferIndex >= CRSF_FRAME_LENGTH) {
            // Check address and type
            if (buffer[0] == CRSF_ADDRESS_FLIGHT_CONTROLLER &&
                buffer[2] == CRSF_FRAME_RC_CHANNELS_PACKED) {

                // Parse 16 channels packed as 11-bit
                uint8_t *payload = &buffer[3]; // skip addr, len, type

                uint32_t bitBuffer = 0;
                uint8_t bitsInBuffer = 0;
                uint8_t channel = 0;

                for (uint8_t i = 0; i < 22; ++i) {
                    bitBuffer |= ((uint32_t)payload[i]) << bitsInBuffer;
                    bitsInBuffer += 8;

                    while (bitsInBuffer >= 11 && channel < 16) {
                        rcChannelValues[channel++] = bitBuffer & 0x7FF;
                        bitBuffer >>= 11;
                        bitsInBuffer -= 11;
                    }
                }

                crsfFailsafe = false; // Valid packet received
            }


            bufferIndex = 0; // reset for next frame
        }
    }
}