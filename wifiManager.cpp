#include "WiFiManager.h"

WiFiManager::WiFiManager(DeviceConfig& config): _config(config) {}

bool WiFiManager::begin() {
    WiFi.mode(WIFI_STA); // Start in STA mode
    WiFi.disconnect(true); // Disconnect from any previous networks
    delay(100);
    Serial.println("WiFiManager: Initialized.");
    return true;
}

// Connects to Wi-Fi using stored credentials
bool WiFiManager::connectWiFi() {
    Serial.print("WiFiManager: Attempting to connect to WiFi: ");
    Serial.println(_config.wifiSsid);

    // Try to connect to STA with a timeout
    if (connectSTA(_config.wifiSsid, _config.wifiPassword, 30000)) { // 30 second timeout
        Serial.print("WiFiManager: Connected to WiFi. IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("WiFiManager: Failed to connect to WiFi.");
        return false;
    }
}

bool WiFiManager::connectSTA(const char* ssid, const char* password, unsigned long timeoutMs) {
    WiFi.begin(ssid, password);
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeoutMs) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(); // Newline after dots
    return WiFi.status() == WL_CONNECTED;
}

// Access Point Mode
void WiFiManager::startAPMode() {
    Serial.println("WiFiManager: Starting AP mode for configuration...");
    WiFi.mode(WIFI_AP);

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String defaultClientId = "Lightbox-" + mac;
    
    WiFi.softAP(defaultClientId, AP_PASSWORD, 1, false, 1);

    Serial.print("WiFiManager: AP SSID: ");
    Serial.println(defaultClientId);
    Serial.print("WiFiManager: AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Start DNS server for captive portal
    _dnsServer.start(53, "*", WiFi.softAPIP()); // Redirect all DNS requests to captive portal IP
}

void WiFiManager::handleDNS() {
    _dnsServer.processNextRequest();
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}
