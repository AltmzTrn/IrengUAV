
#include "CRSFhandler.h"
#include "Attitude_Indicator.h"
#include "actuators.h"
#include "controlSystems.h"
HardwareSerial telemSerial(PA10, PA9);

//variable declaration
bool blinkNow = true;

void setup() {
  pinMode(PC13, OUTPUT);
  telemSerial.begin(115200);
  delay(500);
  telemSerial.println("Initializing IMU...");
  IMU_init();
  telemSerial.println("IMU Ready");
  telemSerial.println("Starting CRSF...");
  crsf_setup();
  telemSerial.println("CRSF Ready");
  actuators_setup();
  //controlSystems_setup();
  telemSerial.println("Actuator Ready");
}

void loop(){
  controlSystems_update();
  
  //print AETR & Attitude
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    telemSerial.print("Failsafe: ");
    telemSerial.print(crsfFailsafe ? "YES" : "NO");
    telemSerial.print(" | CH: ");
    for (int i = 0; i < 4; ++i) // print 4 channels only for example
    {
      telemSerial.print(rcChannelValues[i]);
      telemSerial.print(" ");
    }
    telemSerial.println();

    telemSerial.print("Roll: ");
    telemSerial.print(Attitude[0]);
    telemSerial.print(", Pitch: ");
    telemSerial.print(Attitude[1]);
    telemSerial.print(" , Yaw: ");
    telemSerial.print(Attitude[2]);
    telemSerial.println();
    
    telemSerial.print("desval: ");
    telemSerial.print(desVal[0]);
    telemSerial.print(", ");
    telemSerial.print(desVal[1]);
    telemSerial.print(", ");
    telemSerial.print(desVal[2]);
    telemSerial.println();

    
    telemSerial.print("actval: ");
    telemSerial.print(actVal[0]);
    telemSerial.print(", ");
    telemSerial.print(actVal[1]);
    telemSerial.print(", ");
    telemSerial.print(actVal[2]);
    telemSerial.println();

    telemSerial.println(armChannel);
    telemSerial.println(to_actuator[0]);


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

