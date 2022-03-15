#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <HX711.h>
#include <errno.h>

#include "API.hpp"
#include "AppConfig.hpp"
#include "TempMeasure.hpp"
#include "WeightMeasure.hpp"
#include "MeasurementLog.hpp"
#include "StringUtils.hpp"

AsyncWebServer server(80);
AsyncEventSource events("/api/events");

const char *PARAM_MESSAGE = "message";

struct JsonVariantWrapper
{
    JsonVariant const &json;
};

void API::start()
{
    if (isStarted())
        return;

    Serial.println("Starting async web server...");

    // openapi
    server.on("/api/temperature/history", HTTP_GET, [this](AsyncWebServerRequest *request)
              { getHistoryRecent(request, _temp_history_recent, &_temp_stream_idx); });

    // openapi
    server.on("/api/temperature", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", String(_temp->measureFloat(), 1)); });

    // openapi
    server.on("/api/weight/history", HTTP_GET, [this](AsyncWebServerRequest *request)
              { getHistoryRecent(request, _weight_history_recent, &_weight_stream_idx); });

    // openapi
    server.on("/api/weight/scale", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", String(_weight->scale())); });

    // openapi
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
                  request->send(200, "text/plain", "OK"); });

    // openapi
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

                  app_config.set_weight_scale(_weight->scale());
                  app_config.commit();

                  request->send(200, "text/plain", "OK"); });

    // openapi
    server.on("/api/weight", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", String(_weight->measureFloat(), 1)); });

    // openapi
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
                  request->send(200, "text/plain", "OK"); });

    AsyncCallbackJsonWebHandler *setAPIConfigHandler = new AsyncCallbackJsonWebHandler("/api/config", [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                       { setConfig(request, JsonVariantWrapper{json : json}); });
    setAPIConfigHandler->setMethod(HTTP_POST);
    server.addHandler(setAPIConfigHandler);

    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request)
              { getConfig(request); });

    events.onConnect([](AsyncEventSourceClient *client)
                     {
                         if (client->lastId())
                         {
                             Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
                         } });
    server.addHandler(&events);

    // The web server will serve gzipped files automatically if there's a file ending in '.gz' for the requested resource.
    // It will server '/index.html' by default for '/'.
    server.serveStatic("/", LittleFS, "/web/");

    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, "text/plain", "Not found"); });

    server.begin();
    _is_started = true;
}

void API::getHistoryRecent(AsyncWebServerRequest *request, MeasurementLogOps *log, size_t *curr_idx)
{
    *curr_idx = (size_t)-1;
    AsyncWebServerResponse *response =
        request->beginChunkedResponse("application/json", [this, curr_idx, log](uint8_t *buffer, size_t maxLen, size_t index) -> size_t
                                      {
                                          //Write up to "maxLen" bytes into "buffer" and return the amount written.
                                          //index equals the amount of bytes that have been already sent.
                                          //You will be asked for more data until 0 is returned.

                                          // Serial.printf("tempIdx=%hu size=%hu maxLen=%u index=%u\n", *curr_idx, log->size(), maxLen, index);

                                          if (*curr_idx == size_t(-1))
                                          {
                                              *curr_idx = 0;
                                              return snprintf((char *)buffer, maxLen, "[");
                                          }
                                          if (*curr_idx > log->size())
                                          {
                                              return 0;
                                          }

                                          size_t sz;
                                          if (*curr_idx < log->size())
                                          {
                                              unsigned long t = log->timeMillisAt(*curr_idx);
                                              sz = snprintf((char *)buffer, maxLen, "{\"ts\":%ld,\"value\":%f},", unixTimeAtMillis(t), log->at(*curr_idx));
                                              if (*curr_idx + 1 == log->size())
                                                  sz--; // Remove final ','
                                          }
                                          else
                                          {
                                              sz = snprintf((char *)buffer, maxLen, "]");
                                          }

                                          (*curr_idx)++;
                                          return sz; });

    request->send(response);
}

void API::setConfig(AsyncWebServerRequest *request, JsonVariantWrapper const &json)
{
    /*
        Any of the top-level objects can be omitted. Schema:
        {
            "device_id": <string, read-only>,
            "temp_control_profiles": [
                {
                    "name": <string, max length 47>,
                    "relay_config": {
                        "0": {
                            "kp": <float>,
                            "kd": <float>,
                            "ki": <float>,
                            "ssr": <bool, default false>,
                            "normally_closed": <bool, default false>,
                            "cooling": <bool, default false>
                        }
                    ]
                }
            ],
            "weight_scale": <float>,
            "firebase_auth": {
                "api_key": <string, max length 63>,
                "user_email": <string, max length 255>,
                "user_password": <string, max length 63>
            }
        }
    */
    auto json_obj = json.json.as<JsonObject>();

    auto temp_control_profiles = json_obj["temp_control_profiles"].as<JsonArray>();
    if (!temp_control_profiles.isNull())
    {
        // Serial.printf("got temp_control_profiles count=%d\n", temp_control_profiles.size());

        if (temp_control_profiles.size() > MAX_TEMP_CONTROL_PROFILES)
        {
            request->send(400, "too many temperature control profiles");
            return;
        }

        app_config.set_temp_control_profile_count(temp_control_profiles.size());
        size_t profile_idx = 0;
        for (JsonVariant profile_var : temp_control_profiles)
        {
            auto profile_obj = profile_var.as<JsonObject>();

            TempControlProfile profile{};

            _strlcpy(profile.name, profile_obj["name"].as<String>().c_str());

            auto relay_config_obj = profile_obj["relay_config"].as<JsonObject>();
            for (int i = 0; i < MAX_TEMP_CONTROL_PROFILE_RELAYS; ++i)
            {
                char relay_key[3];
                itoa(i, relay_key, 10);

                auto relay_config = &profile.relay_config[i];

                auto relay_obj = relay_config_obj[relay_key].as<JsonObject>();
                if (relay_obj.isNull())
                {
                    relay_config->active = false;
                    continue;
                }

                // Serial.printf("got temp profile=%s relay=%d relay_key=%s\n", profile.name, i, relay_key);

                relay_config->active = true;
                relay_config->kp = relay_obj["kp"].as<float>();
                relay_config->ki = relay_obj["ki"].as<float>();
                relay_config->kd = relay_obj["kd"].as<float>();
                relay_config->ssr = relay_obj["ssr"].as<bool>();
                relay_config->normally_closed = relay_obj["normally_closed"].as<bool>();
                relay_config->cooling = relay_obj["cooling"].as<bool>();
            }

            app_config.set_temp_control_profile(profile_idx++, profile);
        }
    }

    auto firebase_auth_obj = json_obj["firebase_auth"].as<JsonObject>();
    if (!firebase_auth_obj.isNull())
    {
        FirebaseAuthConfig firebase_auth{};

        _strlcpy(firebase_auth.api_key, firebase_auth_obj["api_key"].as<String>().c_str());
        _strlcpy(firebase_auth.user_email, firebase_auth_obj["user_email"].as<String>().c_str());
        _strlcpy(firebase_auth.user_password, firebase_auth_obj["user_password"].as<String>().c_str());

        app_config.set_firebase_auth(firebase_auth);
    }

    auto weight_scale_var = json_obj["weight_scale"];
    if (!weight_scale_var.isNull())
    {
        app_config.set_weight_scale(weight_scale_var.as<float>());
    }

    app_config.commit();

    request->send(200, "text/plain", "OK");
}

void API::getConfig(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument doc(2048); // should be big enough to fit
    auto root = doc.to<JsonObject>();

    char device_id[9];
    ultoa(app_config.device_id(), device_id, 16);
    root["device_id"] = device_id;

    size_t profile_count = app_config.temp_control_profile_count();
    if (profile_count)
    {
        auto temp_control_profiles = root.createNestedArray("temp_control_profiles");

        for (size_t i = 0; i < profile_count; ++i)
        {
            auto profile_obj = temp_control_profiles.createNestedObject();
            auto profile = app_config.temp_control_profile(i);

            profile_obj["name"] = profile->name;

            auto relay_config_obj = profile_obj.createNestedObject("relay_config");
            for (int j = 0; j < MAX_TEMP_CONTROL_PROFILE_RELAYS; ++j)
            {
                auto relay_config = &profile->relay_config[j];
                if (!relay_config->active)
                    continue;

                char relay_key[3];
                itoa(j, relay_key, 10);

                auto relay_obj = relay_config_obj.createNestedObject(relay_key);
                relay_obj["kp"] = relay_config->kp;
                relay_obj["ki"] = relay_config->ki;
                relay_obj["kd"] = relay_config->kd;
                relay_obj["ssr"] = relay_config->ssr;
                relay_obj["normally_closed"] = relay_config->normally_closed;
                relay_obj["cooling"] = relay_config->cooling;
            }
        }
    }

    root["weight_scale"] = app_config.weight_scale();

    auto firebase_auth = app_config.firebase_auth();
    if (firebase_auth.api_key[0] != 0)
    {
        auto firebase_auth_obj = root.createNestedObject("firebase_auth");
        firebase_auth_obj["user_email"] = firebase_auth.user_email;

        // TODO this is DEBUG ONLY
        firebase_auth_obj["api_key"] = firebase_auth.api_key;
        firebase_auth_obj["user_password"] = firebase_auth.user_password;
    }

    serializeJson(doc, *response);
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

unsigned long API::unixTimeAtMillis(unsigned long t_millis)
{
    return _time_client->getEpochTime() - (millis() - t_millis) / 1000;
}
