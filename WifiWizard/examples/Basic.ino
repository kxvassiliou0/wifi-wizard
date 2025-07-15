#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "WiFiWizard.h"

AsyncWebServer server(80);
WiFiWizard wifiWizard("MyAP"); // Replace "MyAP" with your AP name

void setup()
{
    Serial.begin(115200);

    // Start filesystem for serving HTML, needed for web interface
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed. Starting fallback AP mode...");
        wifiWizard.begin();
        wifiWizard.startAP(); // Start captive portal manually
        return;
    }

    // Begin WiFiWizard (tries to connect to stored network, or starts AP if no creds)
    wifiWizard.begin();

    // Serve static files (like index.html) from LittleFS
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // Optional: Endpoint to scan nearby WiFi networks
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; i++) {
      if (i > 0) json += ",";
      json += "\"" + WiFi.SSID(i) + "\"";
    }
    json += "]";
    request->send(200, "application/json", json); });

    // Handle connect requests
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              {
      StaticJsonDocument<256> doc;
      if (deserializeJson(doc, data, len) == DeserializationError::Ok) {
        String ssid = doc["ssid"].as<String>();
        String password = doc["password"].as<String>();
        wifiWizard.connectToWiFiPublic(ssid, password);
        request->send(200, "application/json", "{\"status\":\"connecting\"}");
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      } });

    server.begin();
    Serial.println("Web server started");
}

void loop()
{
    wifiWizard.loop(); // Keep the WiFiWizard state machine running
}
