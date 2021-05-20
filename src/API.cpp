#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <HX711.h>
#include <errno.h>

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

    // server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
    //     request->send(LittleFS, "/index.html", String(), false, [this](const String &var) -> String {
    //         if (var == "TEMPERATURE")
    //             return String(_temp->currentTemp());
    //         return "UNKNOWN";
    //     });
    // });

    server.on("/temperature/history", HTTP_GET, [this](AsyncWebServerRequest *request) {
        _temp_stream_idx = (uint16_t)-1;

        unsigned long start_time = 0;
        if (request->hasArg("minutes"))
        {
            char *endptr;
            const char *minutes_str = request->arg("minutes").c_str();

            errno = 0;
            unsigned long minutes = strtoul(minutes_str, &endptr, 10);
            if (errno == ERANGE || *endptr != '\0' || minutes_str == endptr)
            {
                request->send(400, "text/plain", "Invalid 'minutes' parameter");
                return;
            }
            
            unsigned long millis_ago = minutes * 60000UL;
            if (millis() >= millis_ago)
                start_time = millis() - millis_ago;
        }

        AsyncWebServerResponse *response = request->beginChunkedResponse("application/json", [this, start_time](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
            //Write up to "maxLen" bytes into "buffer" and return the amount written.
            //index equals the amount of bytes that have been already sent.
            //You will be asked for more data until 0 is returned.

            // Serial.printf("tempIdx=%hu size=%hu maxLen=%u index=%u\n", _temp_stream_idx, _temp_history->size(), maxLen, index);

            if (_temp_stream_idx == uint16_t(-1))
            {
                _temp_stream_idx = 0;
                return snprintf((char *)buffer, maxLen, "{\"ts\":%ld,\"start\":%ld,\"values\":[", millis(), start_time);
            }
            if (_temp_stream_idx > _temp_history->size())
            {
                return 0;
            }

            // Keep the most recent time entries (after start_time)
            for (; _temp_stream_idx < _temp_history->size(); _temp_stream_idx++)
            {
                if (_temp_history->timeMillisAt(_temp_stream_idx) >= start_time)
                    break;
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

    server.on("/weight/raw", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(_load_cell->get_value(5)));
    });

    server.on("/weight/scale", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(_load_cell->get_scale()));
    });

    server.on("/weight", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(_load_cell->get_units(5)));
    });

    server.on("/weight/scale", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("value", true)) {
            request->send(400, "text/plain", "Missing 'value' parameter");
            return;
        }

        char *endptr;
        const char *value_str = request->arg("value").c_str();
        errno = 0;
        float scale = strtof(value_str, &endptr);
        if (errno == ERANGE || *endptr != '\0' || value_str == endptr)
        {
            request->send(400, "text/plain", "Invalid 'value' parameter");
            return;
        }

        _load_cell->set_scale(scale);

        request->send(200, "text/plain", "OK");
    });

    server.on("/weight", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("value", true)) {
            request->send(400, "text/plain", "Missing 'value' parameter");
            return;
        }

        char *endptr;
        const char *value_str = request->arg("value").c_str();
        errno = 0;
        float weight = strtof(value_str, &endptr);
        if (errno == ERANGE || *endptr != '\0' || value_str == endptr)
        {
            request->send(400, "text/plain", "Invalid 'value' parameter");
            return;
        }

        if (weight == 0)
            _load_cell->tare(5);
        else 
            _load_cell->set_offset(weight);

        request->send(200, "text/plain", "OK");
    });

    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId())
        {
            Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
        }
    });
    server.addHandler(&events);

    // The web server will serve gzipped files automatically if there's a file ending in '.gz' for the requested resource.
    // It will server '/index.html' by default for '/'.
    server.serveStatic("/", LittleFS, "/web/");

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

void API::sendCurrentWeight()
{
    char buf[128];
    sprintf(buf, "{\"ts\":%ld,\"value\":%f}", millis(), _load_cell->get_units(5));
    events.send(buf, "weight", millis());
}
