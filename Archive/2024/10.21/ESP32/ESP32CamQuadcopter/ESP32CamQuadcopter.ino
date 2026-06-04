#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"

// Camera pin definitions
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

const char* ssid = "AlBros Sarijadi";           // Your WiFi SSID
const char* password = "02175907227";   // Your WiFi Password

WebServer server(80);  // Create a web server on port 80

// Variables to hold joystick values
int x = 127, y = 127, x2 = 127, y2 = 127;

#define MOTOR_FR    32
#define MOTOR_FL    33
#define MOTOR_RR    0
#define MOTOR_RL    2
int MOTOR_FR_VAL = 0 ;
int MOTOR_FL_VAL = 0 ;
int MOTOR_RR_VAL = 0 ;
int MOTOR_RL_VAL = 0 ;

// Function to initialize the camera
void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Init camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

// Function to handle streaming the camera feed
void handleStream() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(503, "text/plain", "Camera capture failed");
    return;
  }
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

// Define the endpoint for joystick 1
void handleJoystick1() {
  if (server.hasArg("x") && server.hasArg("y")) {
    x = server.arg("x").toInt();
    y = server.arg("y").toInt();
    Serial.print("Joystick 1 - X: ");
    Serial.print(x);
    Serial.print(" Y: ");
    Serial.println(y);

    // Add code here to handle quadcopter movement using x and y values
  }
  server.send(200, "text/plain", "Joystick 1 values received");
}

// Define the endpoint for joystick 2
void handleJoystick2() {
  if (server.hasArg("x2") && server.hasArg("y2")) {
    x2 = server.arg("x2").toInt();
    y2 = server.arg("y2").toInt();
    Serial.print("Joystick 2 - X2: ");
    Serial.print(x2);
    Serial.print(" Y2: ");
    Serial.println(y2); 

    // Add code here to handle quadcopter movement using x2 and y2 values
  }
  server.send(200, "text/plain", "Joystick 2 values received");
}

// Function to serve the main webpage
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Quadcopter Controller</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin: 0;
      padding-top: 20px;
      background-color: #f0f0f0;
    }
    table {
      margin-left: auto;
      margin-right: auto;
    }
    td {
      padding: 8px;
    }
    .button {
      background-color: #000000;
      border: none;
      color: white;
      padding: 10px 20px;
      text-align: center;
      display: inline-block;
      font-size: 18px;
      margin: 6px 3px;
      cursor: pointer;
      user-select: none;
    }
    .joystick-container {
      display: flex;
      justify-content: space-around;
      margin-bottom: 20px;
      flex-wrap: wrap;
    }
    .joystick {
      width: 120px;
      height: 120px;
      background-color: #ddd;
      border-radius: 50%;
      position: relative;
      border: 2px solid #333;
    }
    .joystick-handle {
      width: 40px;
      height: 40px;
      background-color: #333;
      border-radius: 50%;
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
    }
    img {
      width: 100%;
      max-width: 100%;
      height: auto;
      border: 2px solid #333;
      border-radius: 10px;
    }
  </style>
</head>
<body>
  <h1>Personal Unmanned Aerial System</h1>
  
  <!-- Video Stream -->
  <img src="" id="photo" alt="Video Stream">
  
  <!-- Joystick Controls -->
  <div class="joystick-container">
    <div class="joystick" id="joystick1">
      <div class="joystick-handle"></div>
    </div>
    <div class="joystick" id="joystick2">
      <div class="joystick-handle"></div>
    </div>
  </div>
  
  <table>
    <tr><td align="center"><button class="button" onmousedown="toggleCheckbox('forward');" ontouchstart="toggleCheckbox('forward');">Forward</button></td></tr>
    <tr><td align="center"><button class="button" onmousedown="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('stop');">Stop</button></td></tr>
  </table>

  <script>
    // Joystick 1 (x, y values from 0 to 255)
    const joystick1 = document.getElementById('joystick1');
    const joystickHandle1 = joystick1.querySelector('.joystick-handle');
    let joystick1Position = { x: 127, y: 127 };

    joystick1.addEventListener('touchmove', function (event) {
      const touch = event.touches[0];
      const rect = joystick1.getBoundingClientRect();
      let x = touch.clientX - rect.left - joystick1.offsetWidth / 2;
      let y = touch.clientY - rect.top - joystick1.offsetHeight / 2;

      const radius = joystick1.offsetWidth / 2 - joystickHandle1.offsetWidth / 2;
      const distance = Math.sqrt(x * x + y * y);
      
      if (distance > radius) {
        x = (x / distance) * radius;
        y = (y / distance) * radius;
      }

      joystickHandle1.style.transform = `translate(${x}px, ${y}px)`;
      joystick1Position.x = Math.round((x / radius + 1) * 127.5);
      joystick1Position.y = Math.round((y / radius + 1) * 127.5);
      
      sendJoystickData(joystick1Position.x, joystick1Position.y);
    });

    // Keep y value on release
    joystick1.addEventListener('touchend', function () {
      joystickHandle1.style.transform = `translate(-50%, ${joystick1Position.y}px)`;
      joystick1Position.x = 127; // x resets to center
      sendJoystickData(joystick1Position.x, joystick1Position.y);
    });

    // Joystick 2 (x2, y2 values from 0 to 255)
    const joystick2 = document.getElementById('joystick2');
    const joystickHandle2 = joystick2.querySelector('.joystick-handle');
    let joystick2Position = { x: 127, y: 127 };

    joystick2.addEventListener('touchmove', function (event) {
      const touch = event.touches[0];
      const rect = joystick2.getBoundingClientRect();
      let x = touch.clientX - rect.left - joystick2.offsetWidth / 2;
      let y = touch.clientY - rect.top - joystick2.offsetHeight / 2;

      const radius = joystick2.offsetWidth / 2 - joystickHandle2.offsetWidth / 2;
      const distance = Math.sqrt(x * x + y * y);
      
      if (distance > radius) {
        x = (x / distance) * radius;
        y = (y / distance) * radius;
      }

      joystickHandle2.style.transform = `translate(${x}px, ${y}px)`;
      joystick2Position.x = Math.round((x / radius + 1) * 127.5);
      joystick2Position.y = Math.round((y / radius + 1) * 127.5);
      
      sendJoystickData2(joystick2Position.x, joystick2Position.y);
    });

    // Keep y2 value on release
    joystick2.addEventListener('touchend', function () {
      joystickHandle2.style.transform = `translate(-50%, ${joystick2Position.y}px)`;
      joystick2Position.x = 127; // x resets to center
      sendJoystickData2(joystick2Position.x, joystick2Position.y);
    });

    // Send joystick data to the server
    function sendJoystickData(x, y) {
      const xHttp = new XMLHttpRequest();
      xHttp.open("GET", `/joystick1?x=${x}&y=${y}`, true);
      xHttp.send();
    }

    function sendJoystickData2(x2, y2) {
      const xHttp = new XMLHttpRequest();
      xHttp.open("GET", `/joystick2?x2=${x2}&y2=${y2}`, true);
      xHttp.send();
    }

    // Function to refresh the video stream
    function refreshPhoto() {
      const photo = document.getElementById('photo');
      photo.src = '/stream?' + new Date().getTime(); // Add timestamp to avoid caching
    }
    setInterval(refreshPhoto, 100); // Refresh every 100ms
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  startCamera();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi!");

  // Set up server routes
  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.on("/joystick1", handleJoystick1);
  server.on("/joystick2", handleJoystick2);
  server.begin();
  Serial.println("HTTP server started!");
}

void loop() {
  server.handleClient();
  MOTOR_FR_VAL = (0.85*y2)-(0.05*x)+(0.05*y)+(0.05*x2);
  MOTOR_FL_VAL = (0.85*y2)+(0.05*x)+(0.05*y)-(0.05*x2);
  MOTOR_RR_VAL = (0.85*y2)-(0.05*x)-(0.05*y)-(0.05*x2);
  MOTOR_RL_VAL = (0.85*y2)+(0.05*x)-(0.05*y)+(0.05*x2);

  analogWrite(MOTOR_FR, MOTOR_FR_VAL);//put on void loop
  analogWrite(MOTOR_FL, MOTOR_FL_VAL);//put on void loop
  analogWrite(MOTOR_RR, MOTOR_RR_VAL);//put on void loop
  analogWrite(MOTOR_RL, MOTOR_RL_VAL);//put on void loop
}
