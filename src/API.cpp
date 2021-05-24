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
#include "AppConfig.hpp"
#include "TempMeasure.hpp"
#include "WeightMeasure.hpp"
#include "MeasurementLog.hpp"

AsyncWebServer server(80);
AsyncEventSource events("/api/events");

const char *PARAM_MESSAGE = "message";

void API::start()
{
    if (isStarted())
        return;

    Serial.println("Starting async web server...");

    server.on("/api/temperature/history", HTTP_GET, [this](AsyncWebServerRequest *request)
              { getHistoryMinutes(request, _temp_history, &_temp_stream_idx); });

    server.on("/api/temperature", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", String(_temp->measureFloat(), 1)); });

    server.on("/api/weight/history", HTTP_GET, [this](AsyncWebServerRequest *request) {
        getHistoryMinutes(request, _weight_history, &_weight_stream_idx);
    });

    server.on("/api/weight/scale", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", String(_weight->scale())); });

    server.on("/api/weight/scale", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
                  if (!request->hasParam("value", true))
                  {
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

                  _weight->set_scale(scale);
                  request->send(200, "text/plain", "OK");
              });

    server.on("/api/weight/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
                  if (!request->hasParam("value", true))
                  {
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

                  _weight->set_weight(weight);

                  app_config.set_weigth_scale(_weight->scale());
                  app_config.commit();

                  request->send(200, "text/plain", "OK");
              });

    server.on("/api/weight", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", String(_weight->measureFloat(), 1)); });

    server.on("/api/weight", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
                  if (!request->hasParam("value", true))
                  {
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

                  _weight->set_weight_offset(weight);
                  request->send(200, "text/plain", "OK");
              });

    events.onConnect([](AsyncEventSourceClient *client)
                     {
                         if (client->lastId())
                         {
                             Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
                         }
                     });
    server.addHandler(&events);

    // The web server will serve gzipped files automatically if there's a file ending in '.gz' for the requested resource.
    // It will server '/index.html' by default for '/'.
    server.serveStatic("/", LittleFS, "/web/");

    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, "text/plain", "Not found"); });

    server.begin();
    _is_started = true;
}

void API::getHistoryMinutes(AsyncWebServerRequest *request, MeasurementLogOps *log, size_t *curr_idx)
{
    *curr_idx = (size_t)-1;

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

    AsyncWebServerResponse *response =
        request->beginChunkedResponse("application/json", [curr_idx, log, start_time](uint8_t *buffer, size_t maxLen, size_t index) -> size_t
                                      {
                                          //Write up to "maxLen" bytes into "buffer" and return the amount written.
                                          //index equals the amount of bytes that have been already sent.
                                          //You will be asked for more data until 0 is returned.

                                          // Serial.printf("tempIdx=%hu size=%hu maxLen=%u index=%u\n", *curr_idx, log->size(), maxLen, index);

                                          if (*curr_idx == size_t(-1))
                                          {
                                              *curr_idx = 0;
                                              return snprintf((char *)buffer, maxLen, "{\"ts\":%ld,\"start\":%ld,\"values\":[", millis(), start_time);
                                          }
                                          if (*curr_idx > log->size())
                                          {
                                              return 0;
                                          }

                                          // Keep the most recent time entries (after start_time)
                                          for (; *curr_idx < log->size(); (*curr_idx)++)
                                          {
                                              if (log->timeMillisAt(*curr_idx) >= start_time)
                                                  break;
                                          }

                                          size_t sz;
                                          if (*curr_idx < log->size())
                                          {
                                              sz = snprintf((char *)buffer, maxLen, "{\"ts\":%ld,\"value\":%f},", log->timeMillisAt(*curr_idx), log->at(*curr_idx));
                                              if (*curr_idx + 1 == log->size())
                                                  sz--; // Remove final ','
                                          }
                                          else
                                          {
                                              sz = snprintf((char *)buffer, maxLen, "]}");
                                          }

                                          (*curr_idx)++;
                                          return sz;
                                      });

    request->send(response);
}

void API::sendCurrentTemp()
{
    char buf[128];
    sprintf(buf, "{\"ts\":%ld,\"value\":%.1f}", millis(), _temp->measureFloat());
    events.send(buf, "temperature", millis());
}

void API::sendCurrentWeight()
{
    char buf[128];
    sprintf(buf, "{\"ts\":%ld,\"value\":%.1f}", millis(), _weight->measure());
    events.send(buf, "weight", millis());
}
