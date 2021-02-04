#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "API.hpp"
#include "TempMeasure.hpp"

AsyncWebServer server(80);

const char *PARAM_MESSAGE = "message";

void API::start()
{
    if (isStarted())
        return;

    Serial.println("Starting async web server...");

    server.serveStatic("/css", LittleFS, "/css");
    server.serveStatic("/img", LittleFS, "/img");
    server.serveStatic("/js", LittleFS, "/js");

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", String(), false, [this](const String &var) -> String {
            if (var == "TEMPERATURE")
                return String(_temp->currentTemp());
            return "UNKNOWN";
        });
    });

    server.on("/temperature", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(_temp->currentTemp()));
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
        String message;
        if (request->hasParam(PARAM_MESSAGE))
        {
            message = request->getParam(PARAM_MESSAGE)->value();
        }
        else
        {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, GET: " + message);
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
        String message;
        if (request->hasParam(PARAM_MESSAGE, true))
        {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        }
        else
        {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });

    server.begin();
    _isStarted = true;
}
