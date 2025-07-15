#include "WiFiWizard.h"

#define DNS_PORT 53

WiFiWizard::WiFiWizard() : currentState(STATE_OFFLINE)
{
}

void WiFiWizard::begin()
{
    preferences.begin("wifi", false);
    lastSSID = preferences.getString("ssid", "");
    lastPassword = preferences.getString("password", "");
    preferences.end();

    Serial.printf("Stored SSID: %s\n", lastSSID.c_str());

    if (lastSSID != "")
    {
        Serial.println("Attempting to connect to saved WiFi...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(lastSSID.c_str(), lastPassword.c_str());

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 8000)
        {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nConnected to WiFi");
            currentState = STATE_CONNECTED;
            return;
        }
    }

    Serial.println("Starting Access Point Mode...");
    WiFi.mode(WIFI_AP);
    apSSID = "FormaSetup-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.softAP(apSSID.c_str(), NULL);

    delay(500); // Give AP time to settle

    if (!dnsServerStarted)
    {
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        dnsServerStarted = true;
    }

    currentState = STATE_AP_MODE;
}

void WiFiWizard::loop()
{
    static bool inLoop = false;
    if (inLoop)
        return;
    inLoop = true;

    if (currentState == STATE_CONNECTING)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFi connected successfully");
            currentState = STATE_CONNECTED;
        }
        else
        {
            unsigned long now = millis();
            if (now - lastAttemptTime > 10000)
            {
                Serial.println("Retrying WiFi connection...");
                connectToWiFi(lastSSID, lastPassword);
                lastAttemptTime = now;
            }
        }
    }

    if (currentState == STATE_AP_MODE && dnsServerStarted)
    {
        dnsServer.processNextRequest();
    }

    inLoop = false;
}

int WiFiWizard::scanNetworks()
{
    return WiFi.scanNetworks();
}

void WiFiWizard::startAP()
{
    Serial.println("Starting WiFi AP for configuration...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Forma-Setup");

    if (!dnsServerStarted)
    {
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        dnsServerStarted = true;
    }

    currentState = STATE_AP_MODE;
}

void WiFiWizard::stopAPPublic()
{
    if (dnsServerStarted)
    {
        dnsServer.stop();
        dnsServerStarted = false;
    }

    WiFi.softAPdisconnect(true);
    currentState = STATE_OFFLINE;
}

void WiFiWizard::connectToWiFiPublic(const String &ssid, const String &password)
{
    connectToWiFi(ssid, password);
}

void WiFiWizard::connectToWiFi(const String &ssid, const String &password)
{
    Serial.printf("Connecting to WiFi: %s\n", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);

    WiFi.begin(ssid.c_str(), password.c_str());

    currentState = STATE_CONNECTING;
    lastAttemptTime = millis();

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000)
    {
        delay(500);
        Serial.print(".");
        yield();
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi connected.");
        currentState = STATE_CONNECTED;

        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.end();
    }
    else
    {
        Serial.println("Failed to connect.");
        currentState = STATE_OFFLINE;
    }
}

int WiFiWizard::getCurrentState() const
{
    return currentState;
}

void WiFiWizard::processDNS()
{
    if (currentState == STATE_AP_MODE && dnsServerStarted)
    {
        dnsServer.processNextRequest();
    }
}
