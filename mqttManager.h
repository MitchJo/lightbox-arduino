#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> // For TLS/SSL MQTT connection
#include <PubSubClient.h>     // For MQTT client
#include "ConfigManager.h"    // To get MQTT credentials from DeviceConfig

// Define a callback function type for MQTT messages
typedef std::function<void(char* topic, byte* payload, unsigned int length)> MqttCallback;

class MQTTManager {
public:
    // Constructor
    MQTTManager(DeviceConfig& config, MqttCallback callback);

    // Initializes MQTT client with credentials and certificates
    bool begin();

    // Connects to the MQTT broker
    bool connectMQTT();

    // Handles the MQTT client loop (must be called frequently in main loop)
    void loop();

    // Publishes a message to an MQTT topic
    bool publish(const char* topic, const char* payload);

    // Subscribes to an MQTT topic
    bool subscribe(const char* topic);

    // Returns true if connected to the MQTT broker
    bool isConnected();

    void setupTime();

private:
    DeviceConfig& _config;
    WiFiClientSecure _wifiClientSecure;
    PubSubClient _mqttClient;
    MqttCallback _mqttCallback; 

    std::unique_ptr<BearSSL::X509List> _caCert;
    std::unique_ptr<BearSSL::X509List> _clientCert;
    std::unique_ptr<BearSSL::PrivateKey> _privateKey;
    
    unsigned long _lastReconnectAttempt = 0;
    const long _reconnectInterval = 5000; // 5 seconds

    // Callback for incoming MQTT messages
    static void staticCallback(char* topic, byte* payload, unsigned int length);

    // Pointer to the MQTTManager instance for static callback
    static MQTTManager* _instance;
};

#endif
