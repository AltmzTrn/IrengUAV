#include "Attitude_Indicator.h"
#include <Wire.h>
#include <math.h>

#define MPU_ADDR 0x68

int16_t accX, accY, accZ;
int16_t gyroX, gyroY, gyroZ;

float gyroOffset[3] = {0.0f, 0.0f, 0.0f};
float gyroFiltered[3] = {0.0f, 0.0f, 0.0f};
float accelFiltered[3] = {0.0f, 0.0f, 16384.0f}; // Default 1g on Z

float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
int Attitude[3] = {0};
float gyroDegS[3] = {0.0f};

static uint32_t lastUpdateTime = 0;
float dt = 0.0f;

// Filter cutoff frequencies in Hz
static constexpr float GYRO_CUTOFF_HZ = 20.0f;   // Kills brushed motor hum
static constexpr float ACCEL_CUTOFF_HZ = 8.0f;    // Smooths frame acceleration
static constexpr float ALPHA_COMP = 0.98f;        // Complementary filter weight

// Helper for 1st-order PT1 low-pass filter
static inline float applyPT1(float current, float previous, float cutoffHz, float dt) {
  float rc = 1.0f / (2.0f * 3.14159265f * cutoffHz);
  float alpha = dt / (rc + dt);
  return previous + alpha * (current - previous);
}

void IMU_init() {
  Wire.begin();
  Wire.setClock(400000); // Boost I2C speed to 400kHz

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0x00); // Wake up MPU6050
  Wire.endTransmission(true);

  // Enable Hardware DLPF (~20Hz bandwidth for Accel & Gyro)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A); // CONFIG register
  Wire.write(0x04); // 20Hz DLPF setting
  Wire.endTransmission(true);

  delay(100);

  // Calibrate gyro offsets (keep quad completely stationary)
  int32_t gyroSum[3] = {0, 0, 0};
  for (int i = 0; i < 250; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6, true);
    
    gyroSum[0] += (int16_t)((Wire.read() << 8) | Wire.read());
    gyroSum[1] += (int16_t)((Wire.read() << 8) | Wire.read());
    gyroSum[2] += (int16_t)((Wire.read() << 8) | Wire.read());
    delay(2);
  }
  gyroOffset[0] = (gyroSum[0] / 250.0f) / 131.0f;
  gyroOffset[1] = (gyroSum[1] / 250.0f) / 131.0f;
  gyroOffset[2] = (gyroSum[2] / 250.0f) / 131.0f;

  lastUpdateTime = micros();
}

void IMU_update() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  accX = (Wire.read() << 8) | Wire.read();
  accY = (Wire.read() << 8) | Wire.read();
  accZ = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read(); // Skip temp
  gyroX = (Wire.read() << 8) | Wire.read();
  gyroY = (Wire.read() << 8) | Wire.read();
  gyroZ = (Wire.read() << 8) | Wire.read();

  // Loop timing calculation
  uint32_t currentTime = micros();
  if (lastUpdateTime == 0) lastUpdateTime = currentTime;
  dt = (currentTime - lastUpdateTime) * 0.000001f;
  lastUpdateTime = currentTime;
  if (dt <= 0.0f || dt > 0.05f) dt = 0.001f;

  // Convert raw gyro to deg/s and subtract offset
  float rawGyro[3] = {
    (gyroX / 131.0f) - gyroOffset[0],
    (gyroY / 131.0f) - gyroOffset[1],
    (gyroZ / 131.0f) - gyroOffset[2]
  };

  // Software PT1 LPF filtering
  for (uint8_t i = 0; i < 3; i++) {
    gyroFiltered[i] = applyPT1(rawGyro[i], gyroFiltered[i], GYRO_CUTOFF_HZ, dt);
    gyroDegS[i] = gyroFiltered[i];
  }

  accelFiltered[0] = applyPT1((float)accX, accelFiltered[0], ACCEL_CUTOFF_HZ, dt);
  accelFiltered[1] = applyPT1((float)accY, accelFiltered[1], ACCEL_CUTOFF_HZ, dt);
  accelFiltered[2] = applyPT1((float)accZ, accelFiltered[2], ACCEL_CUTOFF_HZ, dt);

  // Calculate angles from filtered accel data
  float accelAngleX = atan2f(accelFiltered[1], accelFiltered[2]) * 57.29578f;
  float accelAngleY = atan2f(-accelFiltered[0], sqrtf(accelFiltered[1] * accelFiltered[1] + accelFiltered[2] * accelFiltered[2])) * 57.29578f;

  // Complementary filter fusion
  angleX = ALPHA_COMP * (angleX + gyroDegS[0] * dt) + (1.0f - ALPHA_COMP) * accelAngleX;
  angleY = ALPHA_COMP * (angleY + gyroDegS[1] * dt) + (1.0f - ALPHA_COMP) * accelAngleY;

  // Yaw integration from filtered gyro Z
  angleZ += gyroDegS[2] * dt;

  if (angleZ > 180.0f) angleZ -= 360.0f;
  else if (angleZ < -180.0f) angleZ += 360.0f;

  Attitude[0] = (int)angleX; // Roll
  Attitude[1] = (int)angleY; // Pitch
  Attitude[2] = (int)angleZ; // Yaw
}