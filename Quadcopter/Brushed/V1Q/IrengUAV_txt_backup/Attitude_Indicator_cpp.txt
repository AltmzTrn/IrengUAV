#include <Wire.h>
#include <math.h>

#define MPU_ADDR 0x68

int16_t accX, accY, accZ;
int16_t gyroX, gyroY, gyroZ;
float angleX, angleY, angleZ;
int Attitude[3] = {0};

unsigned long lastUpdateTime = 0;
float dt = 0;

void IMU_init() {
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1
  Wire.write(0);     // Wake up MPU6050
  Wire.endTransmission(true);

  lastUpdateTime = millis();
}

void IMU_update() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting register for accelerometer
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true); // Accel + Temp + Gyro

  accX = (Wire.read() << 8) | Wire.read();
  accY = (Wire.read() << 8) | Wire.read();
  accZ = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read(); // Skip temperature
  gyroX = (Wire.read() << 8) | Wire.read();
  gyroY = (Wire.read() << 8) | Wire.read();
  gyroZ = (Wire.read() << 8) | Wire.read();

  // Timing
  unsigned long currentTime = millis();
  dt = (currentTime - lastUpdateTime) / 1000.0;
  lastUpdateTime = currentTime;

  // Accelerometer angles
  angleX = atan2(accY, accZ) * 180 / PI;
  angleY = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180 / PI;

  // Integrate gyroZ for yaw (angleZ)
  angleZ += (gyroZ / 131.0) * dt; // 131 = LSB sensitivity for 250dps

  // Clamp angles to [-180, 180]
  if (angleZ > 180) angleZ -= 360;
  else if (angleZ < -180) angleZ += 360;

  Attitude[0] = (int)angleX; // Pitch
  Attitude[1] = (int)angleY; // Roll
  Attitude[2] = (int)angleZ; // Yaw
}
