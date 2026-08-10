#include "controlSystems.h"
#include "CRSFHandler.h"
#include "Attitude_Indicator.h"
#include "actuators.h"

//PID values, not used yet (manual mode first)
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

// toy ESC just wants proportional duty, not a real servo pulse
uint16_t mapCRSFtoDuty(uint16_t crsf_val) {
  return map(constrain(crsf_val, 172, 1811), 172, 1811, 0, 65535);
}


void controlSystems_setup() {
  to_actuator[0] = 0;
  to_actuator[1] = 0;
}

void controlSystems_update() {
  crsf_update(); // Update CRSF
  IMU_update();  // Update IMU, not used yet (manual mode)

  uint16_t throttle = mapControlValuetoPWM(mapCRSFtoDEG(rcChannelValues[2]));
  desVal[0] = mapCRSFtoDEG(rcChannelValues[0]);
  desVal[1] = mapCRSFtoDEG(rcChannelValues[1]);
  desVal[2] = mapCRSFtoDEG(rcChannelValues[3]);

  actVal[0] = Attitude[0];
  actVal[1] = Attitude[1];
  actVal[2] = Attitude[2];

  //Armed state
  armChannel = mapControlValuetoPWM(mapCRSFtoDEG(rcChannelValues[4]));

  if (armChannel > 1500) {
    to_actuator[0] = throttle;                                 // Main Rotor
    to_actuator[1] = map(desVal[2], -180, 180, 0, 65535);       // Tail Rotor, stick proportional for now
  }
  else {
    to_actuator[0] = 1000;                               // Motor
    to_actuator[1] = 0;                               // Tail Rotor
  }

  // Constrain outputs to valid ranges
  to_actuator[0] = constrain(to_actuator[0], 0, 65535);
  to_actuator[1] = constrain(to_actuator[1], 0, 65535);

  actuators_write();
}