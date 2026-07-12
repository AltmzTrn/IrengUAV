#include "controlSystems.h"
#include "CRSFHandler.h"
#include "Attitude_Indicator.h"
#include "actuators.h"

//PID values
//pitch
#define PITCH_P 1
#define PITCH_I 1
#define PITCH_D 0
//roll
#define ROLL_P 0.5
#define ROLL_I 0.5
#define ROLL_D 0
//yaw
#define YAW_P 0.5
#define YAW_I 0.5
#define YAW_D 0

//desire Values
int16_t desVal[3]    = {0};
//actual vlues
int16_t actVal[3] = {0};
//arm channel
uint16_t armChannel = 0;
// write pulse width in microseconds
uint16_t to_actuator[6] = {0};

int16_t mapCRSFtoDEG(uint16_t crsf_val) {
  return map(constrain(crsf_val, 172, 1811), 172, 1811, -180, 180);
}

uint16_t mapControlValuetoPWM(int16_t att) {
  return map(att, -180, 180, 1000, 2000);
}

void controlSystems_update() {
  crsf_update(); // Update CRSF
  IMU_update();  // Update IMU

  // Map CRSF channels to 1000–2000 µs
  uint16_t throttle = mapControlValuetoPWM(mapCRSFtoDEG(rcChannelValues[2])) ;
  desVal[0] = mapCRSFtoDEG(rcChannelValues[0]);
  desVal[1] = mapCRSFtoDEG(rcChannelValues[1]);
  desVal[2] = mapCRSFtoDEG(rcChannelValues[3]);

  // Map attitude to 1000-2000 µs
  int16_t errVal[3] = {0};
  for (uint8_t i=0;i<3;i++) {
    actVal[i] = Attitude[i]; //mapATTtoPWM(Attitude[i]);
    errVal[i] = mapControlValuetoPWM(desVal[i]+actVal[i]) - 1500;
  }

  //stack to PID output value
  int16_t outroll = ROLL_P*errVal[0]+ROLL_I*errVal[0]+ROLL_D*errVal[0];
  int16_t outpitch = PITCH_P*errVal[1]+PITCH_I*errVal[1]+PITCH_D*errVal[1];
  int16_t outyaw = desVal[2];//YAW_P*errVal[2]+YAW_I*errVal[2]+YAW_D*errVal[2];

  //Armed state
  armChannel = mapControlValuetoPWM(mapCRSFtoDEG(rcChannelValues[4]));

  if (armChannel > 1500) {
    to_actuator[0] = 300 + throttle;                      // Motor
    to_actuator[1] = 1500 - outroll + outyaw;            // Yaw Vane Servo 
  }
  else {
    to_actuator[0] = 0;                      // Motor
    to_actuator[1] = 1500 - outyaw;            // Yaw Vane Servo
  }

  // Optional:
  to_actuator[4] = mapCRSFtoDEG(rcChannelValues[5]); // CamAngle
  to_actuator[5] = mapCRSFtoDEG(rcChannelValues[6]); // Laser

  // Constrain outputs to valid PWM range
  for (int j = 0; j < 6; j++) {
    to_actuator[j] = constrain(to_actuator[j], 1000, 2000);
  }


  actuators_write();
}
