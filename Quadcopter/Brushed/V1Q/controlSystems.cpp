#include "controlSystems.h"
#include "CRSFHandler.h"
#include "Attitude_Indicator.h"
#include "actuators.h"

//PID values

//pitch
#define PITCH_P 4.0f
#define PITCH_I 0.8f
#define PITCH_D 0.1f
//roll
#define ROLL_P  4.0f
#define ROLL_I  0.8f
#define ROLL_D  0.1f
//yaw
#define YAW_P   2.0f
#define YAW_I   0.4f
#define YAW_D   0.05f

//desire Values
int16_t desVal[3]    = {0};
//actual vlues
int16_t actVal[3] = {0};
//arm channel
uint16_t armChannel = 0;
// write pulse width in microseconds
uint16_t to_actuator[6] = {0};

int16_t mapCRSFtoDEG(uint16_t crsf_val) {
  return map(constrain(crsf_val, 172, 1811), 172, 1811, -30, 30);
}

uint8_t mapControlValuetoPWM(int16_t att) {
  return map(att, -30, 30, 0, 255);
}

void controlSystems_update() {
  static uint32_t prevtime = 0;
  uint32_t now = millis();
  static float prevErrVal[3] = {0};
  float dt = (now - prevtime) / 1000.0f;
  if (dt <= 0) dt = 1e-3;

  crsf_update(); // Update CRSF
  IMU_update();  // Update IMU

  // Map CRSF channels to 1000–2000 µs
  uint16_t throttle = map(rcChannelValues[2], 172, 1811, 0, 255) ;  
  desVal[0] = mapCRSFtoDEG(rcChannelValues[0]);
  desVal[1] = mapCRSFtoDEG(rcChannelValues[1]);
  desVal[2] = mapCRSFtoDEG(rcChannelValues[3]);

  // Map attitude to 1000-2000 µs
  float errVal[3] = {0};
  for (uint8_t i=0;i<3;i++) {
    actVal[i] = Attitude[i]; 
    errVal[i] = desVal[i];//-actVal[i];
  }

  //stack to PID output value
  int16_t outroll = ROLL_P*errVal[0]+ROLL_I*errVal[0]*dt+ROLL_D*(errVal[0]-prevErrVal[0])/dt;
  int16_t outpitch = PITCH_P*errVal[1]+PITCH_I*errVal[1]*dt+PITCH_D*(errVal[1]-prevErrVal[1])/dt;
  int16_t outyaw = desVal[2];//YAW_P*errVal[2]+YAW_I*errVal[2]+YAW_D*errVal[2];

  //Armed state
  armChannel = mapControlValuetoPWM(mapCRSFtoDEG(rcChannelValues[4]));

  if (armChannel > 128) {
    // Mix for actuators (you can tune the math later)
    to_actuator[0] = throttle*0.4 + mapControlValuetoPWM(0 - outroll*0.4 - outpitch*0.4 - outyaw *0.4);                      // Motor
    to_actuator[1] = throttle*0.4 + mapControlValuetoPWM(0 + outroll*0.4 - outpitch*0.4 + outyaw *0.4);            // Front servo
    to_actuator[2] = throttle*0.4 + mapControlValuetoPWM(0 - outroll*0.4 + outpitch*0.4 + outyaw *0.4);     // Left servo
    to_actuator[3] = throttle*0.4 + mapControlValuetoPWM(0 + outroll*0.4 + outpitch*0.4 - outyaw *0.4);     // Right servo    
  }
  else {
    to_actuator[0] = 0;                      // Motor
    to_actuator[1] = 0;            // Front servo
    to_actuator[2] = 0;     // Left servo
    to_actuator[3] = 0;     // Right servo  
  }

  // Optional:
  to_actuator[4] = mapCRSFtoDEG(rcChannelValues[5]); // CamAngle
  to_actuator[5] = mapCRSFtoDEG(rcChannelValues[6]); // Laser

  // Constrain outputs to valid PWM range
  for (int j = 0; j < 6; j++) {
    to_actuator[j] = constrain(to_actuator[j], 0, 255);
  }


  actuators_write();
  prevtime = now;
  for (uint8_t i = 0;i<3;i++) {
    prevErrVal[i] = errVal[i];
  }

}
