
#include "CRSFhandler.h"
#include "Attitude_Indicator.h"
#include "actuators.h"
#include "controlSystems.h"
//HardwareSerial telemSerial(PA12, PA11);

//variable declaration
bool blinkNow = true;

void setup() {
  pinMode(PC13, OUTPUT);
  Serial.begin(115200);
  delay(500);
  Serial.println("Initializing IMU...");
  IMU_init();
  Serial.println("IMU Ready");
  Serial.println("Starting CRSF...");
  crsf_setup();
  Serial.println("CRSF Ready");
  actuators_setup();
  //controlSystems_setup();
  Serial.println("Actuator Ready");
}

void loop(){
  controlSystems_update();
  
  //print AETR & Attitude
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    Serial.print("Failsafe: ");
    Serial.print(crsfFailsafe ? "YES" : "NO");
   Serial.print(" | CH: ");
    for (int i = 0; i < 4; ++i) // print 4 channels only for example
    {
      Serial.print(rcChannelValues[i]);
      Serial.print(" ");
    }
    Serial.println();

    Serial.print("Roll: ");
    Serial.print(Attitude[0]);
    Serial.print(", Pitch: ");
    Serial.print(Attitude[1]);
    Serial.print(" , Yaw: ");
    Serial.print(Attitude[2]);
    Serial.println();
    
    Serial.print("desval: ");
    Serial.print(desVal[0]);
    Serial.print(", ");
    Serial.print(desVal[1]);
    Serial.print(", ");
    Serial.print(desVal[2]);
    Serial.println();

    
    Serial.print("actval: ");
    Serial.print(actVal[0]);
    Serial.print(", ");
    Serial.print(actVal[1]);
    Serial.print(", ");
    Serial.print(actVal[2]);
    Serial.println();

    Serial.println(armChannel);
    Serial.println(to_actuator[0]);


  }
  
  //Blink
  static unsigned long lastBlinkTime = 0;
  if (millis() - lastBlinkTime > 1000) {
    if (blinkNow) {
      digitalWrite(PC13, HIGH);
      blinkNow = false;
    }
    else {
      digitalWrite(PC13, LOW);
      blinkNow = true;
    }
    lastBlinkTime = millis();
  }
}

