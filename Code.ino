#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// --- WIFI CREDENTIALS ---
const char* ssid = "adm";
const char* password = "antdres67";

WebServer server(80);

// --- SERVO PINS (ESP32-C3 SuperMini) ---
#define PIN_LL 6 // Left Leg
#define PIN_RL 7 // Right Leg
#define PIN_LF 8 // Left Foot
#define PIN_RF 9 // Right Foot
#define Buzzer 3
Servo legL, legR, footL, footR;

String currentCommand = "STOP";
unsigned long previousMillis = 0;
int stepPhase = 0;

// --- HTML PAGE WITH JAVASCRIPT ---
const char html_page[] PROGMEM = R"====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Baymax / Otto Control</title>
    <style>
        body {
            background-color: black;
            font-family: sans-serif;
            color: white;
            text-align: center;
        }
        .title {
            color: crimson;
        }
        .controls {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 12px;
            margin-top: 30px;
        }
        .middle-row {
            display: flex;
            gap: 60px;
        }
        .btn {
            border: none;
            border-radius: 8px;
            padding: 16px 28px;
            background-color: aliceblue;
            color: black;
            font-size: 22px;
            font-weight: bold;
            cursor: pointer;
            min-width: 70px;
        }
        .btn:hover {
            background-color: #d0e4f7;
        }
        .btn:active {
            transform: scale(0.92);
            background-color: crimson;
            color: white;
        }
        .stop-btn {
            background-color: crimson;
            color: white;
            font-size: 18px;
            margin-top: 20px;
        }
    </style>
</head>
<body>

    <h1 class="title">Baymax / Otto Control</h1>
    <div class="controls">
        <button class="btn" onclick="sendCommand('up')">↑</button>
        <div class="middle-row">
            <button class="btn" onclick="sendCommand('left')">←</button>
            <button class="btn" onclick="sendCommand('right')">→</button>
        </div>
        <button class="btn" onclick="sendCommand('down')">↓</button>
    </div>

    <div>
        <button class="btn stop-btn" onclick="sendCommand('stop')">STOP</button>
    </div>

    <script>
        function sendCommand(cmd) {
            fetch('/' + cmd)
                .then(response => console.log('Command sent: ' + cmd))
                .catch(error => console.error('Error:', error));
        }
    </script>

</body>
</html>
)====";

// --- SERVER ROUTE HANDLERS ---
void handleRoot() {
  server.send(200, "text/html", html_page);
}
void handleUp() {
  if (currentCommand != "UP") {
    currentCommand = "UP";
    stepPhase = 0;
    playCommandSound("UP");
  }
  Serial.println("Command: FORWARD");
  server.send(200, "text/plain", "OK");
}
void handleDown() {
  if (currentCommand != "DOWN") {
    currentCommand = "DOWN";
    stepPhase = 0;
    playCommandSound("DOWN");
  }
  Serial.println("Command: BACKWARDS");
  server.send(200, "text/plain", "OK");
}
void handleLeft() {
  if (currentCommand != "LEFT") {
    currentCommand = "RIGHT";
    stepPhase = 0;
    playCommandSound("LEFT");
  }
  Serial.println("Command: TURN LEFT");
  server.send(200, "text/plain", "OK");
}
void handleRight() {
  if (currentCommand != "RIGHT") {
    currentCommand = "LEFT";
    stepPhase = 0;
    playCommandSound("RIGHT");
  }
  Serial.println("Command: TURN RIGHT");
  server.send(200, "text/plain", "OK");
}
void handleStop() {
  if (currentCommand != "STOP") {
    currentCommand = "STOP";
    playCommandSound("STOP");
  }
  legL.write(90);
  legR.write(90);
  footL.write(90);
  footR.write(90);
  Serial.println("Command: STOP");
  server.send(200, "text/plain", "OK");
}
void setupServos() {
  legL.attach(PIN_LL);
  legR.attach(PIN_RL);
  footL.attach(PIN_LF);
  footR.attach(PIN_RF);

  // Home position (90 degrees center)
  legL.write(90);
  legR.write(90);
  footL.write(90);
  footR.write(90);
}

//// sounds////
void playCommandSound(String cmd){
  if (cmd == "UP") {
    tone(Buzzer, 80, 250);
  } else if (cmd == "DOWN") {
    tone(Buzzer, 100, 250);
  } else if (cmd == "LEFT") {
    tone(Buzzer, 90, 250);
  } else if (cmd == "RIGHT") {
    tone(Buzzer, 75, 250);
  } else if (cmd == "STOP") {
    tone(Buzzer, 50, 500);
  }
}
void setup() {
  Serial.begin(115200);
  delay(1000);
  setupServos();
  pinMode(Buzzer, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("Open this IP in your browser: http://");
  Serial.println(WiFi.localIP());

  // Setup Web Server Endpoints
  server.on("/", handleRoot);
  server.on("/up", handleUp);
  server.on("/down", handleDown);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);

  server.begin();
}

void loop() {
  // Handle web server requests continuously
  server.handleClient();

  unsigned long currentMillis = millis();
  // Execute movement loops only based on the current web command
  if (currentCommand == "UP") {
    if (currentMillis - previousMillis >= 200) {
      previousMillis = currentMillis;
      switch (stepPhase) {
        case 0: footL.write(75); footR.write(105); legL.write(75); legR.write(75); stepPhase = 1; break;
        case 1: legR.write(105); stepPhase = 2; break;
        case 2: footL.write(105); footR.write(75); legL.write(105); legR.write(105); stepPhase = 3; break;
        case 3: legL.write(75); stepPhase = 0; break;
      }
    }
  } 
else if (currentCommand == "DOWN") {
  if (currentMillis - previousMillis >= 200) {
    previousMillis = currentMillis;
    switch (stepPhase) {
      case 0: footL.write(105); footR.write(75); legL.write(105); legR.write(105); stepPhase = 1; break;
      case 1: legR.write(75); stepPhase = 2; break;
      case 2: footL.write(75); footR.write(105); legL.write(75); legR.write(75); stepPhase = 3; break;
      case 3: legL.write(105); stepPhase = 0; break;
    }
  }
} 
  else if (currentCommand == "LEFT") {
    if (currentMillis - previousMillis >= 250) {
      previousMillis = currentMillis;
      switch (stepPhase) {
        case 0: footL.write(80); footR.write(80); legL.write(75); legR.write(75); stepPhase = 1; break;
        case 1: footL.write(100); footR.write(100); legL.write(90); legR.write(90); stepPhase = 0; break;
      }
    }
  } 
  else if (currentCommand == "RIGHT") {
    if (currentMillis - previousMillis >= 250) {
      previousMillis = currentMillis;
      switch (stepPhase) {
        case 0: footL.write(80); footR.write(80); legL.write(105); legR.write(105); stepPhase = 1; break;
        case 1: footL.write(100); footR.write(100); legL.write(90); legR.write(90); stepPhase = 0; break;
      }
    }
  }
  // If "STOP", the loop just sits idle while keeping the server running
}
