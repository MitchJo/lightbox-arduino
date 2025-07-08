#include "WebServerHandler.h"

WebServerHandler::WebServerHandler(DeviceConfig& config, std::function<void()> saveConfigCb)
    : _server(HTTP_PORT), _config(config), _saveConfigCb(saveConfigCb) {}

void WebServerHandler::begin() {
    _server.on("/", HTTP_GET, std::bind(&WebServerHandler::handleRoot, this));
    _server.on("/save", HTTP_POST, std::bind(&WebServerHandler::handleSaveConfig, this));
    _server.onNotFound(std::bind(&WebServerHandler::handleNotFound, this));

    _server.begin();
    Serial.println("WebServerHandler: HTTP server started.");
}

void WebServerHandler::handleClient() {
    _server.handleClient();
}

void WebServerHandler::handleRoot() {
    _server.send(200, "text/html", "<h1>Greetings from ESP WebServer</h1>");
}

void WebServerHandler::handleSaveConfig() {

    if (_server.hasArg("wifiSsid")) {
        strlcpy(_config.wifiSsid, _server.arg("wifiSsid").c_str(), sizeof(_config.wifiSsid));
    }
    if (_server.hasArg("wifiPassword")) {
        strlcpy(_config.wifiPassword, _server.arg("wifiPassword").c_str(), sizeof(_config.wifiPassword));
    }

    if(_server.hasArg("mqttHost")){
        strlcpy(_config.mqttHost, _server.arg("mqttHost").c_str(), sizeof(_config.mqttHost));
    } 
    if(_server.hasArg("mqttPort")){
        _config.mqttPort = _server.arg("mqttPort").toInt();
    }

    if(_server.hasArg("mqttCaCert")){
        strlcpy(_config.mqttCaCert, _server.arg("mqttCaCert").c_str(), sizeof(_config.mqttCaCert));
    } 
    if(_server.hasArg("mqttClientCert")){
        strlcpy(_config.mqttClientCert, _server.arg("mqttClientCert").c_str(), sizeof(_config.mqttClientCert));
    }
    if(_server.hasArg("mqttPrivateKey")){
        strlcpy(_config.mqttPrivateKey, _server.arg("mqttPrivateKey").c_str(), sizeof(_config.mqttPrivateKey));
    }

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String defaultClientId = "Lightbox-" + mac;
    strlcpy(_config.mqttClientId, defaultClientId.c_str(), sizeof(_config.mqttClientId));

    _saveConfigCb();

    _server.send(200, "text/plain", "Successfully provisioned.");
    delay(1000);
    ESP.restart(); 
}

void WebServerHandler::handleNotFound() {
    _server.send(404, "text/html", "<h1>Path not Found</h1>");
}
