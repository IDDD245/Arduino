#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <ESPmDNS.h> // Ermöglicht den Aufruf über http://wetter.local

// --- KONFIGURATION ---
const char* ssid = "HPhone"; 
const char* password = "geheim123"; 
// ----------------------

Adafruit_BME280 bme;
WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n--- Wetterstation mit mDNS ---");

    // 1. SENSOR STARTEN
    if (!bme.begin(0x76) && !bme.begin(0x77)) {
        Serial.println("FEHLER: BME280 nicht gefunden!");
    } else {
        Serial.println("Sensor: OK");
    }

    // 2. WLAN VERBINDEN (Optimiert für Samsung Hotspot)
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println("Verbinde mit Hotspot...");
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter < 30) {
        delay(1000);
        
        // Status-Diagnose während des Wartens
        int status = WiFi.status();
        if(status == WL_NO_SSID_AVAIL) Serial.println("Hotspot nicht gefunden (Check 2.4GHz!)");
        else if(status == WL_CONNECT_FAILED) Serial.println("Passwort falsch?");
        else Serial.print(".");
        
        counter++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nVerbunden!");
        Serial.print("IP-Adresse: ");
        Serial.println(WiFi.localIP());

        // 3. mDNS NAME REGISTRIEREN
        // Damit ist die Station unter http://wetter.local erreichbar
        if (MDNS.begin("wetter")) {
            Serial.println("mDNS aktiv: http://wetter.local");
        }
    } else {
        Serial.println("\nTimeout: Verbindung fehlgeschlagen.");
    }

    server.begin();
}

void loop() {
    // mDNS Abfragen verarbeiten (Wichtig für die Namensauflösung)
    // Bei manchen ESP-Versionen ist das nicht nötig, schadet aber nicht.
    
    WiFiClient client = server.available();
    if (client) {
        String request = client.readStringUntil('\r');
        client.flush();

        // JSON DATEN ROUTE
        if (request.indexOf("GET /data") >= 0) {
            client.println("HTTP/1.1 200 OK\nContent-Type: application/json\nConnection: close\n");
            client.printf("{\"temperature\":%.1f,\"humidity\":%.1f,\"pressure\":%.1f}", 
                          bme.readTemperature(), bme.readHumidity(), bme.readPressure() / 100.0F);
        } 
        // HTML SEITE ROUTE
        else {
            client.println("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n");
            client.print("<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>");
            client.print("<style>body{font-family:sans-serif; background:#EBCD67; text-align:center; padding:20px;}");
            client.print(".box{background:white; max-width:400px; margin:auto; padding:20px; border-radius:20px; box-shadow:0 5px 15px rgba(0,0,0,0.2);}");
            client.print("h1{color:#67B8EB; font-family:'Comic Sans MS', cursive;}</style></head><body>");
            client.print("<div class='box'><h1>Wetter Live</h1>");
            client.print("<p id='t' style='font-size:2em;'>Lade...</p>");
            client.print("<p id='h'>...</p>");
            client.print("<script>setInterval(()=>{fetch('/data').then(r=>r.json()).then(d=>{");
            client.print("document.getElementById('t').innerHTML=d.temperature+' &deg;C';");
            client.print("document.getElementById('h').innerHTML='Luftfeuchte: '+d.humidity+' %';");
            client.print("})},2000);</script></div></body></html>");
        }
        client.stop();
    }
}