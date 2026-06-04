/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

int8_t pitchVal = 0;
int8_t rollVal = 0;
uint8_t thrVal = 0;
int8_t yawVal = 0;


#define MOTOR_FR    0
#define MOTOR_FL    33
#define MOTOR_RR    32
#define MOTOR_RL    2



//controls mixing subsystem
//primary motors
int MOTOR_FR_VAL = 0 ;
int MOTOR_FL_VAL = 0 ;
int MOTOR_RR_VAL = 0 ;
int MOTOR_RL_VAL = 0 ;



#include <esp_now.h>
#include <WiFi.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int pitch;
  int roll;
  int pwr;
  int yaw;
} struct_message;

// Create a struct_message called command
struct_message command;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&command, incomingData, sizeof(command));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("pitch: ");
  Serial.println(command.pitch);
  Serial.print("roll: ");
  Serial.println(command.roll);
  Serial.print("pwr: ");
  Serial.println(command.pwr);
  Serial.print("yaw: ");
  Serial.println(command.yaw);
  Serial.println();
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {
  MOTOR_FR_VAL = (0.85*thrVal)-(0.05*rollVal)+(0.05*pitchVal)+(0.05*yawVal);
  MOTOR_FL_VAL = (0.85*thrVal)+(0.05*rollVal)+(0.05*pitchVal)-(0.05*yawVal);
  MOTOR_RR_VAL = (0.85*thrVal)-(0.05*rollVal)-(0.05*pitchVal)-(0.05*yawVal);
  MOTOR_RL_VAL = (0.85*thrVal)+(0.05*rollVal)-(0.05*pitchVal)+(0.05*yawVal);

  analogWrite(MOTOR_FR, MOTOR_FR_VAL);//put on void loop
  analogWrite(MOTOR_FL, MOTOR_FL_VAL);//put on void loop
  analogWrite(MOTOR_RR, MOTOR_RR_VAL);//put on void loop
  analogWrite(MOTOR_RL, MOTOR_RL_VAL);//put on void loop
}