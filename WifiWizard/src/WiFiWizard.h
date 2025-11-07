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

    WiFiWizard(const String &apBaseName = "WifiWizard", const char *apPassword = nullptr, const String &prefsNamespace = "wifi");

    void begin();
    void loop();
    void startAP();

    void startAP(const String &customSSID, const char *password = nullptr);
    void stopAPPublic();
    int scanNetworks();
    void connectToWiFiPublic(const String &ssid, const String &password);
    void processDNS();
    void resetWiFiCredentials();
    int getCurrentState() const;

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
    const char *apPassword = nullptr;
    String preferencesNamespace = "wifi";
    unsigned long lastAttemptTime = 0;

    Preferences preferences;
    DNSServer dnsServer;
    bool dnsServerStarted = false;

    String apBaseName = "WifiWizard";
    void connectToWiFi(const String &ssid, const String &password);
};

#endif
