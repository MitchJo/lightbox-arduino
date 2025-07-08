#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <FS.h>

#define CONFIG_FILE "/config.json"

struct DeviceConfig {
    char wifiSsid[64];
    char wifiPassword[64];

    char mqttHost[128];
    int mqttPort;
    char mqttClientId[64];

    char mqttCaCert[4096];
    char mqttClientCert[2048];
    char mqttPrivateKey[2048];

    bool configured;

    DeviceConfig() : mqttPort(0), configured(false) {
        memset(wifiSsid, 0, sizeof(wifiSsid));
        memset(wifiPassword, 0, sizeof(wifiPassword));

        memset(mqttHost, 0, sizeof(mqttHost));
        memset(mqttClientId, 0, sizeof(mqttClientId));

        memset(mqttCaCert, 0, sizeof(mqttCaCert));
        memset(mqttClientCert, 0, sizeof(mqttClientCert));
        memset(mqttPrivateKey, 0, sizeof(mqttPrivateKey));

    }
};

class ConfigManager {
public:
    static bool begin();

    static bool loadConfig(DeviceConfig& config);

    static bool saveConfig(const DeviceConfig& config);

    static bool clearConfig();

    static bool configExists();
};

#endif