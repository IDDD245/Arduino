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
response += "body { font-family: 'IMPACT', sans-serif; background-color: #fad6a5; color: #333; text-align: center; padding: 20px; }";
response += "h1 { color: #007acc; }";
response += ".container { max-width: 600px; margin: 0 auto; padding: 20px; background: white; border-radius: 10px; box-shadow: 0 2px 20px rgba(0, 0, 0, 0.1); }";
response += ".sensor-data { font-size: 1.5em; margin: 20px 0; padding: 15px; border: 1px solid #007acc; border-radius: 8px; background-color: #f0f8ff; transition: transform 0.3s; }";
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
response += "}, 1000);"; // Aktualisiere alle 2 Sekunden
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
