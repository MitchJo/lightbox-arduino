#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include "ConfigManager.h"

#define AP_PASSWORD "lightb0x123"

class WiFiManager {
public:
    WiFiManager(DeviceConfig& config);

    bool begin();

    void startAPMode();

    bool connectWiFi();

    void handleDNS();

    bool isConnected();

private:
    DNSServer _dnsServer;
    DeviceConfig& _config;
    
    bool connectSTA(const char* ssid, const char* password, unsigned long timeoutMs);
};

#endif
