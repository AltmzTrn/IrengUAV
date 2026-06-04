#include <SPI.h>
#include <LoRa_STM32.h>

#define SS PA4
#define RST PB0
#define DI0 PA1

#define TX_P 17
#define BAND 433E6
#define ENCRYPT 0x78

#define RMR PA8
#define LMR PA9
#define pitchctrl PA10
#define yawctrl PA11

int pwr = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("LoRa Receiver");
  LoRa.setTxPower(TX_P);
  LoRa.setSyncWord(SHANGHAI);

  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
}

void thresholdpwr() {
  while
}


void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}