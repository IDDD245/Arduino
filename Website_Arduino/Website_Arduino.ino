#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "FRITZ!Box 7490_EXT";
const char* password = "43632537225272598463";

// Web server on port 80
WebServer server(80);

// LED pin
const int ledPin = 2;  // default ESP32 built-in LED

// Sensor pin
const int sensorPin = 34;

// ---- HTML PAGE ----
String webpage() {
  return R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Control Panel</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 40px; }
    button { padding: 15px; font-size: 18px; margin: 10px; width: 150px; }
    #sensor { font-size: 24px; color: blue; margin-top: 20px; }
  </style>
</head>
<body>
  <h1>ESP32 Control Panel</h1>

  <button onclick="fetch('/on')">LED ON</button>
  <button onclick="fetch('/off')">LED OFF</button>
  <button onclick="fetch('/aaa')">LED AAAA</button>

  <h2>Sensor Value:</h2>
  <div id="sensor">---</div>

  <script>
    function updateSensor() {
      fetch('/sensor')
        .then(response => response.text())
        .then(value => { document.getElementById('sensor').innerHTML = value; });
    }
    setInterval(updateSensor, 1000); // update every second
  </script>
</body>
</html>
)=====";
}

// ---- HANDLERS ----

void handleRoot() {
  server.send(200, "text/html", webpage());
}

void handleOn() {
  digitalWrite(ledPin, HIGH);
  Serial.println("Luftfeuchtigkeit: ");
  server.send(200, "text/plain", "LED ON");
}

     void handleAaa() {
      
       Serial.println("aaa: ");
       server.send(200, "text/plain", "LED ON");
     }

void handleOff() {
  digitalWrite(ledPin, LOW);
  server.send(200, "text/plain", "LED OFF");
}

void handleSensor() {
  int value = analogRead(sensorPin);
  server.send(200, "text/plain", String(value));
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Route setup
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/aaa", handleAaa);
  server.on("/sensor", handleSensor);

  server.begin();
}

void loop() {
  server.handleClient();
}
