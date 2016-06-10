#include "setup.h"

#include <ArduinoJson.h>
#include "config.h"
#include "httpServer.h"
#include "wifi.h"
#include "ota.h"
#include "file.h"

class IRKit irkit;

void IRKit::setup(void) {
  wdt_reset();

  //getHostname();

  setupFile();
  settingsRestoreFromFile();

  switch (mode) {
    case IR_STATION_MODE_NULL:
      println_dbg("Boot Mode: NULL");
      configureAccesspoint(hostname, "");
      break;
    case IR_STATION_MODE_STA:
      println_dbg("Boot Mode: Station");
      connectCachedWifi();
      attachInterrupt(PIN_IR_IN, irExternalISR, CHANGE);
      break;
  }
  setupServer();
}

void IRKit::setMode(uint8_t newMode) {
  mode = newMode;
  settingsBackupToFile();
}

bool IRKit::serialParser(String serial) {
  int index = 0;
  index = serial.indexOf('/', index + 1);
  index = serial.indexOf('/', index + 1);
  index = serial.indexOf('/', index + 1);
  irkit.devicekey = serial.substring(index + 1, serial.indexOf('/', index + 1));
  println_dbg("devicekey: " + irkit.devicekey);
  return true;
}

void IRKit::settingsRestoreFromFile() {
  String s;
  if (getStringFromFile(SETTINGS_DATA_PATH, s) == false) return;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(s);
  mode = (int)data["mode"];
  devicekey = (const char*)data["devicekey"];
  clienttoken = (const char*)data["clienttoken"];
}

void IRKit::settingsBackupToFile() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["mode"] = mode;
  data["devicekey"] = devicekey;
  data["clienttoken"] = clienttoken;
  String str;
  data.printTo(str);
  writeStringToFile(SETTINGS_DATA_PATH, str);
}


