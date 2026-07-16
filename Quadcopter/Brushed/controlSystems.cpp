#include "controlSystems.h"
#include "CRSFHandler.h"
#include "Attitude_Indicator.h"
#include "actuators.h"

//armed state
bool armedState=true;

//PID values
//pitch
static constexpr float PITCH_P = 0.25f;
static constexpr float PITCH_I = 0.05f;
static constexpr float PITCH_D = 0.00625f;
//roll
static constexpr float ROLL_P = 0.25f;
static constexpr float ROLL_I = 0.05f;
static constexpr float ROLL_D = 0.00625f;
//yaw
static constexpr float YAW_P  = 0.0625f;
static constexpr float YAW_I  = 0.0125f;
static constexpr float YAW_D  = 0.003125f;
static constexpr int16_t YAW_RATE_MAX = 6; // max yaw rate in deg s^-1

//Initialise all Control Outputs
int16_t outroll = 0;
int16_t outpitch = 0;
int16_t outyaw = 0;

//desire Values
int16_t desVal[3] = {0};
//actual vlues
int16_t actVal[3] = {0};
//arm channel
uint16_t armChannel = 0;
//flight mode channel
uint16_t modeChannel = 0;
//default flight mode
char fltMode = 'M';
// write pulse width in microseconds
uint16_t to_actuator[6] = {0};

static int16_t mapCRSFtoRange(uint16_t crsf_val, int16_t outMin, int16_t outMax) {
  return map(constrain(crsf_val, 172, 1811), 172, 1811, outMin, outMax);
}

static int16_t mapCRSFtoDEG(uint16_t crsf_val) {
  return mapCRSFtoRange(crsf_val, -30, 30);
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

  IMU_update(); // Update State
  crsf_update(); // Update Input
  //Armed state
  armChannel = mapControlValuetoPWM(mapCRSFtoDEG(rcChannelValues[4]));
  //Flight Mode
  modeChannel = rcChannelValues[5];


  // Map CRSF channels to 1000–2000 µs
  uint16_t throttle = map(rcChannelValues[2], 172, 1811, 0, 255) ;  
  desVal[0] = mapCRSFtoDEG(rcChannelValues[0]); //desired roll angle in deg
  desVal[1] = mapCRSFtoDEG(rcChannelValues[1]); //desired pitch angle in deg
  desVal[2] = mapCRSFtoDEG(rcChannelValues[3]); //desired yaw rate in deg s^-1

  //map Attitude to 1000–2000 µs
  actVal[0] = Attitude[0];
  actVal[1] = Attitude[1];
  actVal[2] = Attitude[2];

  // Map attitude to 1000-2000 µs
  float errVal[3] = {0};
  errVal[0] = desVal[0]-actVal[0];
  errVal[1] = desVal[1]-actVal[1];
  errVal[2] = desVal[2]-gyroDegS[2];

  //integral for the PID controller
  static float Integral[3] = {0};
  for (uint8_t i = 0; i<3; i++) {
    Integral[i] = constrain(Integral[i]+errVal[i]*dt,-50.0f,50.0f);
  } 

  //Flight Mode Switching + stack to PID output value for stabilize
  if (modeChannel<720) {
    fltMode = 'M'; //manual
    int16_t outroll = desVal[0];
    int16_t outpitch = desVal[1];
    int16_t outyaw = desVal[2];
  }
  else if (modeChannel>720 && modeChannel<1265) {
    fltMode = 'A'; //Attitude Hold
  }
  else {
    fltMode = 'S'; //Stabilise
    int16_t outroll = ROLL_P*errVal[0]+ROLL_I*Integral[0]+ROLL_D*(errVal[0]-prevErrVal[0])/dt;
    int16_t outpitch = PITCH_P*errVal[1]+PITCH_I*Integral[1]+PITCH_D*(errVal[1]-prevErrVal[1])/dt;
    int16_t outyaw = YAW_P*errVal[2]+YAW_I*Integral[2]+YAW_D*(errVal[2]-prevErrVal[2])/dt;
  }

  if (armChannel > 128) {
    // Mix for actuators (you can tune the math later)
    to_actuator[0] = throttle*0.4 + mapControlValuetoPWM(0 - outroll*0.4 - outpitch*0.4 - outyaw *0.4);    // Front Right Motor
    to_actuator[1] = throttle*0.4 + mapControlValuetoPWM(0 + outroll*0.4 - outpitch*0.4 + outyaw *0.4);    // Front Left Motor
    to_actuator[2] = throttle*0.4 + mapControlValuetoPWM(0 - outroll*0.4 + outpitch*0.4 + outyaw *0.4);    // Rear Right Motor
    to_actuator[3] = throttle*0.4 + mapControlValuetoPWM(0 + outroll*0.4 + outpitch*0.4 - outyaw *0.4);    // Rear Left Motor    
    armedState = true;
  }
  else {
    to_actuator[0] = 0;
    to_actuator[1] = 0;
    to_actuator[2] = 0;
    to_actuator[3] = 0;
    armedState = false;
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
