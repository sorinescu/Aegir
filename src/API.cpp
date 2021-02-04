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
#include "TempHistory.hpp"

AsyncWebServer server(80);
AsyncEventSource events("/events");

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

    server.on("/temperature/history", HTTP_GET, [this](AsyncWebServerRequest *request) {
        _temp_stream_idx = (uint16_t)-1;

        AsyncWebServerResponse *response = request->beginChunkedResponse("application/json", [this](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
            //Write up to "maxLen" bytes into "buffer" and return the amount written.
            //index equals the amount of bytes that have been already sent.
            //You will be asked for more data until 0 is returned.

            // Serial.printf("tempIdx=%hu size=%hu maxLen=%u index=%u\n", _temp_stream_idx, _temp_history->size(), maxLen, index);

            if (_temp_stream_idx == uint16_t(-1))
            {
                _temp_stream_idx = 0;
                return snprintf((char *)buffer, maxLen, "{\"ts\":%ld,\"values\":[", millis());
            }
            if (_temp_stream_idx > _temp_history->size())
            {
                return 0;
            }

            size_t sz;
            if (_temp_stream_idx < _temp_history->size())
            {
                sz = snprintf((char *)buffer, maxLen, "{\"ts\":%ld,\"value\":%f},", _temp_history->timeMillisAt(_temp_stream_idx), _temp_history->at(_temp_stream_idx));
                if (_temp_stream_idx + 1 == _temp_history->size())
                    sz--; // Remove final ','
            }
            else
            {
                sz = snprintf((char *)buffer, maxLen, "]}");
            }

            _temp_stream_idx++;
            return sz;
        });

        request->send(response);
    });

    server.on("/temperature", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(_temp->currentTemp()));
    });

    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId())
        {
            Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
        }
    });
    server.addHandler(&events);

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
    _is_started = true;
}

void API::sendCurrentTemp()
{
    char buf[128];
    sprintf(buf, "{\"ts\":%ld,\"value\":%f}", millis(), _temp->currentTemp());
    events.send(buf, "temperature", millis());
}
