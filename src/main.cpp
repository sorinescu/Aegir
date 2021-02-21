/**
 * WiFiManager advanced demo, contains advanced configurartion options
 * Implements TRIGGEN_PIN button press, press for ondemand configportal, hold for 3 seconds for reset settings.
 */
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <EasyButton.h>
#include <WiFiManager.h>
#include "API.hpp"
#include "TempMeasure.hpp"
#include "TempHistory.hpp"

#define BUTTON_PIN 0
#define BAUDRATE 115200

// #define TEMP_PIN D4 // LoLin board
#define TEMP_PIN 0     // Flash pin (WiFi rubber ducky board)

#define TEMP_SAMPLE_CAPACITY 4000
#define TEMP_SAMPLE_INTERVAL_MILLIS 5000

WiFiManager wm;                    // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params )
EasyButton button(BUTTON_PIN);
PositiveTempMeasure temp(TEMP_PIN);
TempHistory temp_history(&temp, 12, TEMP_SAMPLE_CAPACITY, TEMP_SAMPLE_INTERVAL_MILLIS);
API api(&temp, &temp_history);

static void onPressedOnce()
{
  Serial.println("Button pressed");

  // start portal w delay
  Serial.println("Starting config portal");
  wm.setConfigPortalTimeout(120);

  if (!wm.startConfigPortal("OnDemandAP", "password"))
  {
    Serial.println("failed to connect or hit timeout");
    delay(3000);
    ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }
}

static void onPressedForDuration()
{
  Serial.println("Button long press");

  Serial.println("Erasing Config, restarting");
  wm.resetSettings();
  ESP.restart();
}

static String getParam(String name)
{
  //read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name))
  {
    value = wm.server->arg(name);
  }
  return value;
}

static void saveParamCallback()
{
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}

void setup()
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(BAUDRATE);
  Serial.setDebugOutput(true);
  delay(3000);
  Serial.println("\n Starting");

  // Initialize the temperature sensor
  temp.setup();

  // Initialize the button.
  button.begin();
  button.onPressedFor(2000, onPressedForDuration);
  button.onPressed(onPressedOnce);

  // wm.resetSettings(); // wipe settings

  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"");

  // test custom html input type(checkbox)
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\""); // custom html type

  // test custom html(radio)
  const char *custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input

  wm.addParameter(&custom_field);
  wm.setSaveParamsCallback(saveParamCallback);
  wm.setConfigPortalBlocking(false);

  // custom menu via array or vector
  //
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"};
  // wm.setMenu(menu,6);
  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "restart", "exit"};
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  //set static ip
  // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
  // wm.setShowStaticFields(true); // force show static ip fields
  // wm.setShowDnsFields(true);    // force show dns field always

  // wm.setConnectTimeout(20); // how long to try to connect for before continuing
  wm.setConfigPortalTimeout(30); // auto close configportal after n seconds
  // wm.setCaptivePortalEnable(false); // disable captive portal redirection
  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
  // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
  // wm.setShowInfoErase(false);      // do not show erase button on info page
  // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons

  // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

  //automatically connect using saved credentials if they exist
  //If connection fails it starts an access point with the specified name
  if (wm.autoConnect("AutoConnectAP"))
  {
    Serial.println("connected...yeey :)");
  }
  else
  {
    Serial.println("Configportal running");
  }

  LittleFS.begin();

  // OTA updates
  ArduinoOTA.setHostname("Aegir");
  ArduinoOTA.onStart([]() {
    LittleFS.end();
    Serial.println("Start OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd OTA");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loop()
{
  ArduinoOTA.handle();

  // Continuously read the status of the button.
  button.read();

  // Update WiFi manager state
  wm.process();

  // Start web server
  if (!api.isStarted() && WiFi.status() == WL_CONNECTED)
  {
    api.start();
  }

  // Read temperature continuously
  temp.loop();

  // Collect periodic temp samples in a circular buffer
  bool new_temp_sample = temp_history.collect();

  // Serial.printf("Temp history @ %ld: ", temp_history.startTimeMillis());
  // for (uint16_t i=0; i<temp_history.size(); i++) {
  //   Serial.printf("%.2f ", temp_history.at(i));
  // }
  // Serial.println();
  // delay(500);

  if (new_temp_sample && api.isStarted())
    api.sendCurrentTemp();
}