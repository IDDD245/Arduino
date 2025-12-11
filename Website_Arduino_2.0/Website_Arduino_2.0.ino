#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h> // Für ESP32

Adafruit_BME280 bme; // I2C
const char* ssid = "A15 von Owen"; // WLAN SSID
const char* password = "Amber111"; // WLAN Passwort

WiFiServer server(80);

void setup() {
    Serial.begin(115200);

    // BME280 starten
    if (!bme.begin(0x76)) {
        Serial.println("Konnte keinen gültigen BME280 Sensor finden!");
        while (1);
    }

    // WLAN verbinden
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Verbinde mit WLAN...");
    }

    server.begin();
    Serial.println("Server gestartet");
    Serial.print("IP Adresse: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Überprüfen, ob ein Client verbunden ist
    WiFiClient client = server.available();
    if (client) {
        String request = client.readStringUntil('\r');
        client.flush();

        if (request.indexOf("/data") != -1) { // Anforderung der Sensordaten
            // Werte aus dem BME280 lesen
            float temperature = bme.readTemperature();
            float humidity = bme.readHumidity();
            float pressure = bme.readPressure() / 100.0F; // hPa

            // JSON-Antwort erstellen
            String jsonResponse = "{";
            jsonResponse += "\"temperature\": " + String(temperature) + ",";
            jsonResponse += "\"humidity\": " + String(humidity) + ",";
            jsonResponse += "\"pressure\": " + String(pressure);
            jsonResponse += "}";

            // HTTP Antwort senden
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();
            client.println(jsonResponse);
        } else { // Standard-HTML-Seite
            String response = "<html><head><meta charset='UTF-8'><style>";
            response += "body { font-family: 'IMPACT', Aral; background-color: #fad6a5; color: #333; text-align: center; display: flex; justify-content: center; align-items: center; height: 100vh; }";
            response += ".container { max-width: 700px; width: 100%; padding: 40px; background: white; border-radius: 20px; box-shadow: 0 4px 40px rgba(0, 0, 0, 0.1); text-align: center; }";
            response += "h1 { font-size: 3em; color: #0000e6; }"; // Überschrift auf 3em setzen
            response += ".sensor-data { font-size: 2em; margin: 40px 0; padding: 30px; border: 1px solid #007acc; border-radius: 16px; background-color: #f0f8ff; transition: transform 0.3s; }";
            response += ".sensor-data:hover { transform: scale(1.05); }";
            response += "</style></head><body>";
            response += "<div class='container'>";
            response += "<h1>BME280 Sensor Daten</h1>";
            response += "<p class='sensor-data' id='temperature'>Temperatur: -- &deg;C</p>";
            response += "<p class='sensor-data' id='humidity'>Luftfeuchtigkeit: -- %</p>";
            response += "<p class='sensor-data' id='pressure'>Luftdruck: -- hPa</p>";
            response += "<script>";
            response += "setInterval(function() {";
            response += "fetch('/data').then(response => response.json()).then(data => {";
            response += "document.getElementById('temperature').innerHTML = 'Temperatur: ' + data.temperature + ' &deg;C';";
            response += "document.getElementById('humidity').innerHTML = 'Luftfeuchtigkeit: ' + data.humidity + ' %';";
            response += "document.getElementById('pressure').innerHTML = 'Luftdruck: ' + data.pressure + ' hPa';";
            response += "}).catch(error => console.log('Fehler:', error));";
            response += "}, 1000);"; // Aktualisiere alle 1 Sekunde
            response += "</script>";
            response += "</div></body></html>";

           // HTTP Antwort senden
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println(response);
        }
        client.stop(); // Verbindung schließen
    }
}


