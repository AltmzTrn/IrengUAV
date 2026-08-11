#include "CRSFHandler.h"

HardwareSerial crsfSerial(PB11, PB10); //Other uarts already for other stuff

int rcChannelValues[crsfProtocol::RC_CHANNEL_COUNT] = {0};
bool crsfFailsafe = true;

#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8
#define CRSF_FRAME_RC_CHANNELS_PACKED  0x16
#define CRSF_FRAME_LENGTH              26 // 1 (addr) + 1 (len) + 1 (type) + 22 (payload)
#define CRSF_FAILSAFE_TIMEOUT_MS       300 // no valid frame in this long, assume link lost

static uint8_t buffer[CRSF_FRAME_LENGTH];
static uint8_t bufferIndex = 0;
static uint32_t lastFrameTime = 0;

static uint8_t crsf_crc8(uint8_t crc, uint8_t a) {
    crc ^= a;
    for (uint8_t i = 0; i < 8; ++i) {
        crc = (crc & 0x80) ? (crc << 1) ^ 0xD5 : (crc << 1);
    }
    return crc;
}

void crsf_setup() {
    crsfSerial.begin(420000, SERIAL_8N1);
}

void crsf_update() {
    while (crsfSerial.available()) {
        uint8_t byteIn = crsfSerial.read();

        // resync: don't start a frame on anything but a valid address byte
        if (bufferIndex == 0 && byteIn != CRSF_ADDRESS_FLIGHT_CONTROLLER) {
            continue;
        }

        buffer[bufferIndex++] = byteIn;

        // frame validation
        if (bufferIndex >= CRSF_FRAME_LENGTH) {
            // type + CRC over type/payload (bytes 2..24), checked against byte 25
            uint8_t crc = 0;
            for (uint8_t i = 2; i < CRSF_FRAME_LENGTH - 1; ++i) {
                crc = crsf_crc8(crc, buffer[i]);
            }

            if (buffer[2] == CRSF_FRAME_RC_CHANNELS_PACKED && crc == buffer[CRSF_FRAME_LENGTH - 1]) {

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

                crsfFailsafe = false;
                lastFrameTime = millis();
            }

            bufferIndex = 0; 
        }
    }

    // no good frame in a while, link is actually lost
    if (millis() - lastFrameTime > CRSF_FAILSAFE_TIMEOUT_MS) {
        crsfFailsafe = true;
    }
}