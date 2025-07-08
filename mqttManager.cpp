#include "MQTTManager.h"

// Initialize static instance pointer
MQTTManager* MQTTManager::_instance = nullptr;

MQTTManager::MQTTManager(DeviceConfig& config, MqttCallback callback)
    : _config(config),
      _mqttClient(_wifiClientSecure), // Initialize PubSubClient with WiFiClientSecure
      _mqttCallback(callback) {
    _instance = this; // Set the static instance pointer
}

void MQTTManager::setupTime(){
    Serial.println("MQTTManager: setting up the time...");

    // configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    time_t now = time(nullptr);
    int attempts = 0;
    const int maxAttempts = 30; 

    while (now < 100000 && attempts < maxAttempts) {
        delay(500); 
        Serial.print(".");
        now = time(nullptr);
        attempts++;
    }

    if (now < 100000) {
        Serial.println("\nTime synchronization failed after max attempts!");
        Serial.println("Please check WiFi connection and NTP server availability.");
        return; 
    }

    Serial.println("\nTime synchronized!");


    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);

    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
  
}

// Initializes MQTT client with credentials and certificates
bool MQTTManager::begin() {
    Serial.println("MQTTManager: Initializing TLS and MQTT client...");
    setupTime();

    // Set certificates for secure client
    if (strlen(_config.mqttCaCert) > 0) {
        _caCert = std::make_unique<BearSSL::X509List>(_config.mqttCaCert);
        _wifiClientSecure.setTrustAnchors(_caCert.get());

        Serial.println("MQTTManager: CA certificate set.");
    } else {
        Serial.println("MQTTManager: No CA certificate provided. TLS might fail if server needs validation.");
    }

    if (strlen(_config.mqttClientCert) > 0 && strlen(_config.mqttPrivateKey) > 0) {

        _clientCert = std::make_unique<BearSSL::X509List>(_config.mqttClientCert);
        _privateKey = std::make_unique<BearSSL::PrivateKey>(_config.mqttPrivateKey);

        _wifiClientSecure.setClientRSACert(_clientCert.get(), _privateKey.get());

        Serial.println("MQTTManager: Client certificate and private key set.");
    } else {
        Serial.println("MQTTManager: No client certificate/private key provided. Mutual TLS might fail.");
    }

    // Set MQTT server and port
    _mqttClient.setServer(_config.mqttHost, _config.mqttPort);
    // Set the callback for incoming messages
    _mqttClient.setCallback(MQTTManager::staticCallback);

    Serial.println("MQTTManager: MQTT client initialized.");
    return true;
}

// Static callback function to route to the class instance method
void MQTTManager::staticCallback(char* topic, byte* payload, unsigned int length) {
    if (_instance && _instance->_mqttCallback) {
        _instance->_mqttCallback(topic, payload, length);
    }
}

// Connects to the MQTT broker
bool MQTTManager::connectMQTT() {
    if (_mqttClient.connected()) {
        return true; // Already connected
    }

    Serial.print("MQTTManager: Attempting MQTT connection to ");
    Serial.print(_config.mqttHost);
    Serial.print(":");
    Serial.println(_config.mqttPort);

    // Attempt to connect with credentials
    bool connected;

    connected = _mqttClient.connect(_config.mqttClientId);
    
    if (connected) {
        Serial.println("MQTTManager: Connected to MQTT broker.");
        return true;
    } else {
        Serial.print("MQTTManager: MQTT connection failed, rc=");
        Serial.print(_mqttClient.state());
        Serial.println(". Will try again...");
        // You might want to log the PubSubClient state codes for debugging:
        // -4: MQTT_CONNECTION_TIMEOUT
        // -3: MQTT_CONNECTION_LOST
        // -2: MQTT_CONNECT_FAILED
        // -1: MQTT_DISCONNECTED
        //  0: MQTT_CONNECTED
        //  1: MQTT_CONNECT_BAD_PROTOCOL
        //  2: MQTT_CONNECT_BAD_CLIENT_ID
        //  3: MQTT_CONNECT_UNAVAILABLE
        //  4: MQTT_CONNECT_BAD_CREDENTIALS
        //  5: MQTT_CONNECT_UNAUTHORIZED
        return false;
    }
}

// Handles the MQTT client loop
void MQTTManager::loop() {
    if (!_mqttClient.connected()) {
        unsigned long currentMillis = millis();
        if (currentMillis - _lastReconnectAttempt > _reconnectInterval) {
            _lastReconnectAttempt = currentMillis;
            // Attempt to reconnect
            if (!connectMQTT()) {
                // If reconnection fails, do not proceed with client loop until next interval
                return;
            }
        } else {
            return; // Not time to reconnect yet
        }
    }
    _mqttClient.loop(); // Process incoming/outgoing MQTT messages
}

// Publishes a message
bool MQTTManager::publish(const char* topic, const char* payload) {
    if (!_mqttClient.connected()) {
        Serial.println("MQTTManager: Not connected to MQTT broker. Cannot publish.");
        return false;
    }
    Serial.print("MQTTManager: Publishing to topic '");
    Serial.print(topic);
    Serial.print("': ");
    Serial.println(payload);
    return _mqttClient.publish(topic, payload);
}

// Subscribes to a topic
bool MQTTManager::subscribe(const char* topic) {
    if (!_mqttClient.connected()) {
        Serial.println("MQTTManager: Not connected to MQTT broker. Cannot subscribe.");
        return false;
    }
    Serial.print("MQTTManager: Subscribing to topic: ");
    Serial.println(topic);
    return _mqttClient.subscribe(topic);
}

// Returns true if connected
bool MQTTManager::isConnected() {
    return _mqttClient.connected();
}
