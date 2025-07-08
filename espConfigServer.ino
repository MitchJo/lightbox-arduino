#include "WiFiManager.h"
#include "WebServerHandler.h"
#include "ConfigManager.h"
#include "MqttManager.h"
#include "ledManager.h"

#define RESET_PIN 4

WiFiManager *wiFiManagerPtr;
WebServerHandler *webServerHandlerPtr;
MQTTManager *mqttManagerPtr;
DeviceConfig deviceConfig;

bool inConfigMode = false;


void mqttCallback(char* topic, byte* payload, unsigned int msg_length){
  Serial.print("MQTTManager: Message arrived on topic: [");
  Serial.print(topic);
  Serial.println("]");
  
  if(msg_length <= 128) { 
    LEDManager::parsePayload(payload, msg_length); 
  }
  
}

void setup()
{

  Serial.begin(9600);
  pinMode(RESET_PIN, INPUT_PULLUP);

  LEDManager::begin();

  if (!ConfigManager::begin()){
    Serial.println("ERROR: Failed to initialize LittleFS. Cannot proceed.");
    while (true){
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
    }
  }

  if (ConfigManager::loadConfig(deviceConfig)){
    Serial.println("Setup: Configuration loaded from LittleFS.");
  }else{
    Serial.println("Setup: No valid configuration found or failed to load. Entering configuration mode.");
    inConfigMode = true;
  }

  wiFiManagerPtr = new WiFiManager(deviceConfig);
  mqttManagerPtr = new MQTTManager(deviceConfig, mqttCallback);

  webServerHandlerPtr = new WebServerHandler(deviceConfig,[](){
    if(!ConfigManager::saveConfig(deviceConfig)){
      Serial.println("Cannot save config");
    }
  });

  if(inConfigMode){
    
    wiFiManagerPtr->startAPMode();
    webServerHandlerPtr->begin();

  }else{

      LEDManager::setRGBStatus(0,0,255);

      if (wiFiManagerPtr->connectWiFi()) {
        mqttManagerPtr->begin();

        if (mqttManagerPtr->connectMQTT()) {
          LEDManager::setRGBStatus(0,255,0);
          Serial.println("Setup: MQTT Connected. Ready to operate!");
          mqttManagerPtr->subscribe("lightbox/command");
        } else {
          LEDManager::setRGBStatus(255,0,30);
          Serial.println("Setup: Failed to connect to MQTT. Check credentials/broker.");
        }

      }else{
        Serial.println("Cannot connect to wifi...");
        LEDManager::setRGBStatus(240,240,0);
      }

  }

}

void loop(){

  if (digitalRead(RESET_PIN) == LOW) {
    if(ConfigManager::clearConfig()) {
      delay(1000);
      ESP.restart();
    }
  }
  
  if(inConfigMode){

    webServerHandlerPtr->handleClient();
    wiFiManagerPtr->handleDNS();

  }else{

    if (!wiFiManagerPtr->isConnected()) {
      Serial.println("Loop: Wi-Fi lost. Attempting to reconnect...");
      if (!wiFiManagerPtr->connectWiFi()) {
        Serial.println("Loop: Failed to reconnect Wi-Fi. Restarting to re-enter config mode.");
        digitalWrite(LED_BUILTIN, LOW); // Turn LED on to indicate WiFi loss
        delay(5000); // Wait a bit
        ESP.restart(); // Restart to fall back to config mode
      }
    }


    if (wiFiManagerPtr->isConnected()) {
      mqttManagerPtr->loop();
    }

  }

}
