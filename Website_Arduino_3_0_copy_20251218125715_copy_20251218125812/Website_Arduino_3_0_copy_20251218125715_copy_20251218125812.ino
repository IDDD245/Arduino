#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <ESPmDNS.h>

// --- DEINE HOTSPOT DATEN ---
const char* ssid = "HPhone";      // Dein Handy-Netzwerkname
const char* password = "geheim123";      // Dein Handy-Passwort
// ---------------------------

Adafruit_BME280 bme; 
WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    delay(2000); // Zeit zum Öffnen des Seriellen Monitors
    Serial.println("\n--- Initialisierung Owen's Wetterstation ---");

    // 1. SENSOR STARTEN (Checkt beide I2C Adressen)
    if (!bme.begin(0x76) && !bme.begin(0x77)) {
        Serial.println("FEHLER: BME280 Sensor nicht gefunden!");
        Serial.println("Prüfe: SDA -> Pin 21, SCL -> Pin 22");
    } else {
        Serial.println("Sensor: OK");
    }

    // 2. WLAN VERBINDEN (Optimiert für Samsung/Handy Hotspots)
    WiFi.mode(WIFI_STA); 
    WiFi.begin(ssid, password);

    Serial.print("Verbinde mit Hotspot");
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 40) {
        delay(500);
        Serial.print(".");
        
        // Kleine Diagnose während der Verbindung
        if (timeout == 20) {
            Serial.println("\n[Info] Verbindung dauert lange... 2.4 GHz am Handy aktiv?");
        }
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nVERBUNDEN!");
        Serial.print("IP-Adresse: ");
        Serial.println(WiFi.localIP());

        // 3. mDNS STARTEN (Erreichbar unter http://wetter.local)
        if (MDNS.begin("wetter")) {
            Serial.println("Hostname registriert: http://wetter.local");
        }
    } else {
        Serial.println("\nFEHLER: Verbindung fehlgeschlagen.");
        Serial.println("TIPP: Am Samsung Hotspot 'Band' auf '2,4 GHz' stellen!");
    }

    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    
    if (client) {
        String request = client.readStringUntil('\r');
        client.flush();

        // ROUTE 1: JSON Daten für das automatische Update
        if (request.indexOf("GET /data") >= 0) {
            float t = bme.readTemperature();
            float h = bme.readHumidity();
            float p = bme.readPressure() / 100.0F;

            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();
            client.printf("{\"temperature\":%.1f,\"humidity\":%.1f,\"pressure\":%.1f}", t, h, p);
        } 
        // ROUTE 2: Die Webseite selbst (HTML/CSS)
        else {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            client.print("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
            client.print("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
            client.print("<style>");
            client.print("body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #67B8EB, #EBCD67); margin: 0; display: flex; align-items: center; justify-content: center; min-height: 100vh; color: #333; }");
            client.print(".card { background: rgba(255, 255, 255, 0.95); padding: 60px; border-radius: 30px; box-shadow: 0 30px 70px rgba(0,0,0,0.2); width: 85%; max-width: 800px; text-align: center; backdrop-filter: blur(20px); }");
            client.print("h1 { font-size: 3.6em; margin-bottom: 50px; color: #007acc; font-family: 'Comic Sans MS', cursive; }");
            client.print(".data-row { display: flex; justify-content: space-between; align-items: center; background: white; margin: 20px 0; padding: 30px 40px; border-radius: 30px; box-shadow: inset 0 4px 10px rgba(0,0,0,0.05); border: 2px solid #eee; }");
            client.print(".label { font-weight: bold; color: #666; font-size: 3.0em;}");
            client.print(".value { font-size: 2.8em; color: #007acc; font-weight: 900; }");
            client.print("</style></head><body>");
            client.print("<div class='card'><h1>Hannes und Owen Wetterstation</h1>");
            client.print("<div class='data-row'><span class='label'>Temperatur</span><span class='value' id='t'>-- &deg;C</span></div>");
            client.print("<div class='data-row'><span class='label'>Luftfeuchtigkeit</span><span class='value' id='h'>-- %</span></div>");
            client.print("<div class='data-row'><span class='label'>Luftdruck</span><span class='value' id='p'>-- hPa</span></div>");
            client.print("<script>");
            client.print("function update() { fetch('/data').then(r=>r.json()).then(d=>{");
            client.print("document.getElementById('t').innerHTML=d.temperature.toFixed(1)+' &deg;C';");
            client.print("document.getElementById('h').innerHTML=d.humidity.toFixed(1)+' %';");
            client.print("document.getElementById('p').innerHTML=Math.round(d.pressure)+' hPa';");
            client.print("}).catch(e=>console.error('Fehler:', e)); }");
            client.print("setInterval(update, 1000); update();");
            client.print("</script></div></body></html>");
        }
        client.stop();
    }
}