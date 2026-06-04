#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <LoRa.h>

// WiFi credentials
const char* ssid = "Tired Workaholic's GSM Router";
const char* password = "244466666";

// LoRa settings
#define LORA_SCK 12
#define LORA_MISO 13
#define LORA_MOSI 11
#define LORA_SS 10
#define LORA_RST 5
#define LORA_DIO0 4
#define BAND 433E6
#define TELEMETRY_SIZE 32
#define COMMAND_SIZE 8

// Global variables
WebServer server(80);
char telemetry[TELEMETRY_SIZE];
char command[COMMAND_SIZE];
int roll = 0, pitch = 0, yaw = 0, throttle = 0;
bool isArmed = false;

// HTML for the GCS webserver interface
const char INDEX_HTML[] PROGMEM = R"rawliteral(
rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Ireng GCS</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; background: #f0f0f0; }
    canvas { touch-action: none; }
    .joystick { width: 150px; height: 150px; background: #ddd; border: 2px solid #333; border-radius: 50%; margin: 20px auto; position: relative; }
    .handle { width: 50px; height: 50px; background: #333; border-radius: 50%; position: absolute; transform: translate(-50%, -50%); }
    .button { margin: 20px auto; padding: 10px 20px; background: #000; color: #fff; border: none; font-size: 18px; cursor: pointer; }
    .telemetry { margin-top: 20px; font-size: 16px; }
  </style>
</head>
<body>
  <h1>Ireng GCS</h1>
  <div id="telemetry" class="telemetry">Waiting for telemetry...</div>
  <div id="leftJoystick" class="joystick">
    <div id="leftHandle" class="handle"></div>
  </div>
  <div id="rightJoystick" class="joystick">
    <div id="rightHandle" class="handle"></div>
  </div>
  <button class="button" onclick="toggleArm()">ARM/DISARM</button>
  <script>
    const telemetryDiv = document.getElementById('telemetry');
    let armed = false;

    // Joystick settings
    const leftJoystick = document.getElementById('leftJoystick');
    const leftHandle = document.getElementById('leftHandle');
    const rightJoystick = document.getElementById('rightJoystick');
    const rightHandle = document.getElementById('rightHandle');

    const joystickSettings = {
      left: { x: 0, y: 255, defaultY: 255 },
      right: { x: 127, y: 127, defaultX: 127, defaultY: 127 }
    };

    function sendJoystickData() {
      fetch(`/command?roll=${joystickSettings.right.x}&pitch=${joystickSettings.right.y}&yaw=${joystickSettings.left.x}&throttle=${joystickSettings.left.y}`)
        .catch(err => console.error('Error sending data:', err));
    }

    function handleJoystickMove(event, joystick, handle, settings) {
      const rect = joystick.getBoundingClientRect();
      const x = event.touches[0].clientX - rect.left - joystick.offsetWidth / 2;
      const y = event.touches[0].clientY - rect.top - joystick.offsetHeight / 2;

      const radius = joystick.offsetWidth / 2 - handle.offsetWidth / 2;
      const distance = Math.sqrt(x * x + y * y);

      if (distance > radius) {
        x = (x / distance) * radius;
        y = (y / distance) * radius;
      }

      handle.style.transform = `translate(${x}px, ${y}px)`;

      settings.x = Math.round((x / radius + 1) * 127.5);
      settings.y = Math.round((y / radius + 1) * 127.5);

      sendJoystickData();
    }

    leftJoystick.addEventListener('touchmove', (e) => handleJoystickMove(e, leftJoystick, leftHandle, joystickSettings.left));
    rightJoystick.addEventListener('touchmove', (e) => handleJoystickMove(e, rightJoystick, rightHandle, joystickSettings.right));

    function toggleArm() {
      armed = !armed;
      fetch(`/arm?armed=${armed}`).catch(err => console.error('Error sending arm command:', err));
    }

    // Fetch telemetry periodically
    function fetchTelemetry() {
      fetch('/telemetry')
        .then(response => response.json())
        .then(data => {
          telemetryDiv.innerHTML = `Roll: ${data.roll}, Pitch: ${data.pitch}, Yaw: ${data.yaw}, Altitude: ${data.altitude}`;
        })
        .catch(err => console.error('Error fetching telemetry:', err));
    }

    setInterval(fetchTelemetry, 500); // Fetch telemetry every 500ms
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

void handleTelemetry() {
    server.send(200, "text/plain", telemetry);
}

void handleCommand() {
    if (server.hasArg("data")) {
        String data = server.arg("data");
        if (data == "ARM") {
            isArmed = !isArmed;
            sprintf(command, "%c", isArmed ? 'A' : 'D');
        } else {
            sscanf(data.c_str(), "%d,%d,%d,%d", &roll, &pitch, &yaw, &throttle);
            sprintf(command, "%c,%d,%d,%d,%d", isArmed ? 'A' : 'D', roll, pitch, yaw, throttle);
        }
        server.send(200, "text/plain", "Command received");
        // Debugging information
        Serial.printf("Command sent: %s\n", command);
    } else {
        server.send(400, "text/plain", "Invalid command");
    }
}

void setup() {
    Serial.begin(115200);

    // WiFi setup
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    // LoRa setup
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(BAND)) {
        Serial.println("LoRa init failed!");
        while (1);
    }

    // Webserver routes
    server.on("/", handleRoot);
    server.on("/telemetry", handleTelemetry);
    server.on("/command", handleCommand);

    server.begin();
    Serial.println("HTTP server started");
    Serial.print(WiFi.localIP());

}

void loop() {
    server.handleClient();

    // Send command to UAV
    LoRa.beginPacket();
    LoRa.write((uint8_t*)command, COMMAND_SIZE);
    LoRa.endPacket();

    // Receive telemetry from UAV
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        int index = 0;
        while (LoRa.available() && index < TELEMETRY_SIZE) {
            telemetry[index++] = LoRa.read();
        }
        telemetry[index] = '\0';
        Serial.printf("Telemetry received: %s\n", telemetry);  // Debugging telemetry
    }
}
