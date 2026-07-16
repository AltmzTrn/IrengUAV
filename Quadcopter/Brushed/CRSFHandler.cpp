#include "CRSFHandler.h"

int rcChannelValues[crsfProtocol::RC_CHANNEL_COUNT] = {0};
bool crsfFailsafe = true;

#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8
#define CRSF_FRAME_RC_CHANNELS_PACKED  0x16
#define CRSF_FRAME_LENGTH              26 // 1 (addr) + 1 (len) + 1 (type) + 22 (payload) + 1 (crc)

static uint8_t buffer[CRSF_FRAME_LENGTH];
static uint8_t bufferIndex = 0;

#define CRSF_RXBUF_SIZE 64
static volatile uint8_t rxBuf[CRSF_RXBUF_SIZE];
static volatile uint8_t rxHead = 0;
static volatile uint8_t rxTail = 0;

extern "C" void USART3_IRQHandler(void) {
    if (USART3->SR & USART_SR_RXNE) {
        uint8_t b = USART3->DR; // reading DR returns the byte AND clears RXNE
        uint8_t next = (rxHead + 1) % CRSF_RXBUF_SIZE;
        if (next != rxTail) { // drop the byte if the ring buffer is full, don't overwrite unread data
            rxBuf[rxHead] = b;
            rxHead = next;
        }
    }
}

void crsf_setup() {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    GPIOB->CRH &= ~((0xFu << 8) | (0xFu << 12));
    GPIOB->CRH |=  (0xBu << 8) | (0x4u << 12); // PB10 AF push-pull 50MHz (TX), PB11 floating input (RX)

    USART3->BRR = 86; // ~420000 baud @ 36MHz PCLK1 (~0.33% error)
    USART3->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART3_IRQn);
}

void crsf_update() {
    while (rxTail != rxHead) {
        uint8_t byteIn = rxBuf[rxTail];
        rxTail = (rxTail + 1) % CRSF_RXBUF_SIZE;

        buffer[bufferIndex++] = byteIn;

        // frame validation
        if (bufferIndex >= CRSF_FRAME_LENGTH) {
            // address and type
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

                crsfFailsafe = false;
            }


            bufferIndex = 0; 
        }
    }
}