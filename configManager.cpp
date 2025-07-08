#include "ConfigManager.h"
#include <LittleFS.h> 
#include <ArduinoJson.h> 

bool ConfigManager::begin() {
    if (!LittleFS.begin()) {
        Serial.println("ConfigManager: An error occurred while mounting LittleFS.");
        return false;
    }
    Serial.println("ConfigManager: LittleFS mounted successfully.");
    return true;
}

bool ConfigManager::loadConfig(DeviceConfig& config) {
    if (!configExists()) {
        Serial.println("ConfigManager: Configuration file does not exist. Starting with default values.");
        config.configured = false; // Mark as not configured
        return false;
    }

    File configFile = LittleFS.open(CONFIG_FILE, "r");
    if (!configFile) {
        Serial.println("ConfigManager: Failed to open config file for reading.");
        config.configured = false;
        return false;
    }

    size_t size = configFile.size();
    if (size > 8192) { // Max size for JSON config (8KB for certificates)
        Serial.println("ConfigManager: Configuration file size is too large!");
        configFile.close();
        config.configured = false;
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    configFile.close();

    // Use StaticJsonDocument for small JSON, DynamicJsonDocument for larger/unknown size.
    // With certificates, DynamicJsonDocument is safer.
    // The memory needed for the DynamicJsonDocument is estimated. For large certs, this might need tuning.
    // An approximate calculation: 2x (cert size) + string overhead.
    // For 2KB certs, 4KB CA, plus other strings, 12KB-15KB might be needed.
    DynamicJsonDocument doc(16384); // 16KB should be sufficient for certs and other params

    DeserializationError error = deserializeJson(doc, buf.get());

    if (error) {
        Serial.print("ConfigManager: Failed to parse config file: ");
        Serial.println(error.f_str());
        config.configured = false;
        return false;
    }

    // Populate the struct from JSON
    // WiFi
    strlcpy(config.wifiSsid, doc["wifiSsid"] | "", sizeof(config.wifiSsid));
    strlcpy(config.wifiPassword, doc["wifiPassword"] | "", sizeof(config.wifiPassword));

    strlcpy(config.mqttHost, doc["mqttHost"] | "", sizeof(config.mqttHost));
    config.mqttPort = doc["mqttPort"] | 8883;
    strlcpy(config.mqttClientId, doc["mqttClientId"] | "", sizeof(config.mqttClientId));

    strlcpy(config.mqttCaCert, doc["mqttCaCert"] | "", sizeof(config.mqttCaCert));
    strlcpy(config.mqttClientCert, doc["mqttClientCert"] | "", sizeof(config.mqttClientCert));
    strlcpy(config.mqttPrivateKey, doc["mqttPrivateKey"] | "", sizeof(config.mqttPrivateKey));

    // Check if essential configurations are present
    config.configured = (strlen(config.wifiSsid) > 0 &&
                         strlen(config.mqttHost) > 0 &&
                         config.mqttPort != 0);

    Serial.println("ConfigManager: Configuration loaded successfully.");
    if (config.configured) {
        Serial.println("ConfigManager: Device is configured.");
    } else {
        Serial.println("ConfigManager: Device is NOT fully configured (missing essential data).");
    }
    return true;
}

bool ConfigManager::saveConfig(const DeviceConfig& config) {
    DynamicJsonDocument doc(16384); // Allocate enough memory

    doc["wifiSsid"] = config.wifiSsid;
    doc["wifiPassword"] = config.wifiPassword;

    doc["mqttHost"] = config.mqttHost;
    doc["mqttClientId"] = config.mqttClientId;
    doc["mqttPort"] = config.mqttPort;

    doc["mqttClientCert"] = config.mqttClientCert;
    doc["mqttCaCert"] = config.mqttCaCert;
    doc["mqttPrivateKey"] = config.mqttPrivateKey;


    File configFile = LittleFS.open(CONFIG_FILE, "w");
    if (!configFile) {
        Serial.println("ConfigManager: Failed to open config file for writing.");
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println("ConfigManager: Failed to write to config file.");
        configFile.close();
        return false;
    }
    
    configFile.close();
    Serial.println("ConfigManager: Configuration saved successfully.");
    return true;
}

bool ConfigManager::clearConfig() {
    if (LittleFS.remove(CONFIG_FILE)) {
        Serial.println("ConfigManager: Configuration file cleared.");
        return true;
    } else {
        Serial.println("ConfigManager: Failed to clear configuration file.");
        return false;
    }
}

bool ConfigManager::configExists() {
    return LittleFS.exists(CONFIG_FILE);
}