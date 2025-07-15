#ifndef WiFiWizard_h
#define WiFiWizard_h

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <Preferences.h>

class WiFiWizard
{
public:
    WiFiWizard();

    void begin();        // Initialize WiFiWizard
    void loop();         // Call repeatedly in your main loop
    void startAP();      // Start Access Point mode with captive portal
    void stopAPPublic(); // Stop Access Point mode and DNS server
    int scanNetworks();  // Scan available WiFi networks
    void connectToWiFiPublic(const String &ssid, const String &password);
    void processDNS();           // Call in loop for captive DNS
    int getCurrentState() const; // Get current state (enum)

private:
    enum WiFiState
    {
        STATE_AP_MODE = 0,
        STATE_OFFLINE = 1,
        STATE_CONNECTING = 2,
        STATE_CONNECTED = 3
    };

    int currentState;
    String lastSSID;
    String lastPassword;
    String apSSID;
    unsigned long lastAttemptTime = 0;

    Preferences preferences;
    DNSServer dnsServer;
    bool dnsServerStarted = false;

    void connectToWiFi(const String &ssid, const String &password);
};

#endif
