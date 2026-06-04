#include "controlSystems.h"
#include "CRSFHandler.h"
#include "Attitude_Indicator.h"
#include "actuators.h"

// write pulse width in microseconds
uint16_t control_value[7] = {0};
uint16_t to_actuator[6] = {0};

uint16_t mapCRSFtoPWM(uint16_t crsf_val) {
  return map(constrain(crsf_val, 172, 1811), 172, 1811, 1000, 2000);
}

void controlSystems_update() {
  crsf_update(); // Update CRSF
  IMU_update();  // Update IMU

  // Map CRSF channels to 1000–2000 µs
  uint16_t throttle = mapCRSFtoPWM(rcChannelValues[2]) ;
  int16_t roll    = mapCRSFtoPWM(rcChannelValues[0]) - 1500;
  int16_t pitch     = mapCRSFtoPWM(rcChannelValues[1]) - 1500;
  int16_t yaw      = mapCRSFtoPWM(rcChannelValues[3]) - 1500;

  // Mix for actuators (you can tune the math later)
  to_actuator[0] = 300 + throttle;                      // Motor
  to_actuator[1] = 1500 - roll + yaw;            // Front servo
  to_actuator[2] = 1500 + roll - pitch + yaw;     // Left servo
  to_actuator[3] = 1500 + roll + pitch + yaw;     // Right servo

  // Optional:
  to_actuator[4] = mapCRSFtoPWM(rcChannelValues[4]); // CamAngle
  to_actuator[5] = mapCRSFtoPWM(rcChannelValues[7]); // Laser

  // Constrain outputs to valid PWM range
  for (int j = 0; j < 6; j++) {
    to_actuator[j] = constrain(to_actuator[j], 1000, 2000);
  }

  actuators_write();
}
