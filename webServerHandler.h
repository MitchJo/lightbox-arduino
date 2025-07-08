#ifndef WEB_SERVER_HANDLER_H
#define WEB_SERVER_HANDLER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "ConfigManager.h"

// Define the port for the web server
#define HTTP_PORT 80

class WebServerHandler {
public:
    // Constructor takes a reference to the config struct and a save callback
    // WebServerHandler(DeviceConfig& config, std::function<void()> saveCallback);
    WebServerHandler(DeviceConfig& config, std::function<void()> saveConfigCb);

    // Starts the web server and defines routes
    void begin();

    // Handles incoming web requests (must be called frequently in main loop)
    void handleClient();

private:
    ESP8266WebServer _server;
    DeviceConfig& _config;
    std::function<void()> _saveConfigCb;

    // Handler for the root path (serves the config form)
    void handleRoot();

    // Handler for saving the configuration
    void handleSaveConfig();

    // Handler for unknown paths
    void handleNotFound();
};

#endif
