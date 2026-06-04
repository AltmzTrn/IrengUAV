/*
  Master Node

*/
#include <SPI.h>              // include libraries
#include <LoRa.h>

#define TX_P 17
#define BAND 433E6
#define ENCRYPT 0x78

#define PITCH 3
#define ROLL 5
#define POWER 6
#define YAW 9
int dof = 4;

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages -- the syntax 'byte' is to assign a variable to an 8-bit unsigned decimal number
byte localAddress = 0xBB;     // address of this device -- 0xBB = 187? -- the initial 0x means a hexadecimal number
byte destination = 0xFF;      // destination to send to -- 0xFF = 255
long lastSendTime = 0;        // last send time
int interval = 50;          // interval between sends

void setup() {
  LoRa.setTxPower(TX_P);
  LoRa.setSyncWord(SHANGHAI);
  Serial.begin(115200);                   // initialize serial
  pinMode(pitch, INPUT);
  pinMode(roll, INPUT);
  pinMode(power, INPUT);
  pinMode(yaw, INPUT);
  while (!Serial);

  Serial.println("LoRa Duplex");

  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
}

void loop() {
  if (millis() - lastSendTime > interval) {
    int transmitarray[6] = {analogRead(pitch), analogRead(roll), analogRead(power), analogRead(yaw), 0, 0}; 
    TXTransmitter(transmitarray);
   // Serial.println("Sending " + message);
   delay(5);
  }
  TXReceive(LoRa.parsePacket());
}

void TXTransmitter(int outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
//  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void TXReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
   // Serial.println("error: message length does not match length");
   ;
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    //Serial.println("This message is not for me.");
    ;
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
 // Serial.println("Received from: 0x" + String(sender, HEX));
 // Serial.println("Sent to: 0x" + String(recipient, HEX));
  //Serial.println("Message ID: " + String(incomingMsgId));
 // Serial.println("Message length: " + String(incomingLength));
  Serial.println(incoming);
  //Serial.println("RSSI: " + String(LoRa.packetRssi()));
  //Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

