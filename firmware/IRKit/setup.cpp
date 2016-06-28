#include "setup.h"

#include <ArduinoJson.h>
#include "config.h"
#include "httpServer.h"
#include "wifi.h"
#include "ota.h"
#include "file.h"
#include "CRC8.h"

class IRKit irkit;
class IR_SIGNAL signal(PIN_IR_OUT, PIN_IR_IN);

void IRKit::setup(void) {
  wdt_reset();

  generateHostname();

  setupFile();
  if (settingsRestoreFromFile() == false) reset();

  setupButtonInterrupt();

#if USE_OTA_UPDATE == true
  setupOTA();
#endif

  switch (mode) {
    case IRKIT_MODE_NULL:
      println_dbg("Boot Mode: NULL");
      WiFi.mode(WIFI_AP_STA);
      configureAccesspoint(hostname, WIFI_AP_PASSWORD);
      setupAPServer();
      break;
    case IRKIT_MODE_STA:
      println_dbg("Boot Mode: Station");
      WiFi.mode(WIFI_STA);
      connectWifi(ssid, password);
      setupServer();
      attachInterrupt(signal.rxPin, []() {
        signal.isr();
      }, CHANGE);
      break;
  }
}

void IRKit::reset() {
  ssid = "";
  password = "";
  devicekey = "";
  clienttoken = "";
  setMode(IRKIT_MODE_NULL);
  ESP.reset();
}

void IRKit::setMode(uint8_t newMode) {
  mode = newMode;
  settingsBackupToFile();
}

void IRKit::setupButtonInterrupt() {
  attachInterrupt(PIN_BUTTON, []() {
    static uint32_t prev_ms;
    if (digitalRead(PIN_BUTTON) == LOW) {
      prev_ms = millis();
      println_dbg("the button pressed");
    } else {
      println_dbg("the button released");
      if (millis() - prev_ms > 2000) {
        println_dbg("the button long pressed");
        irkit.reset();
      }
    }
  }, CHANGE);
  println_dbg("attached button interrupt");
}

void IRKit::generateHostname() {
  char hex[] = "0123456789ABCDEF";
  hostname = "IRKit";
  hostname += hex[(ESP.getChipId() >> 12) & 0xF];
  hostname += hex[(ESP.getChipId() >> 8) & 0xF];
  hostname += hex[(ESP.getChipId() >> 4) & 0xF];
  hostname += hex[(ESP.getChipId() >> 0) & 0xF];
  print_dbg("Chip ID: ");
  println_dbg(ESP.getChipId(), HEX);
  print_dbg("Hostname: ");
  println_dbg(hostname);
}

static char x2c(char highByte, char lowByte) {
  if (highByte >= '0' && highByte <= '9') {
    highByte -= '0';
  } else if (highByte >= 'A' && highByte <= 'F') {
    highByte -= 'A';
    highByte += 0xA;
  } else if (highByte >= 'a' && highByte <= 'f') {
    highByte -= 'a';
    highByte += 0xA;
  } else {
    return -1;
  }
  if (lowByte >= '0' && lowByte <= '9') {
    lowByte -= '0';
  } else if (lowByte >= 'A' && lowByte <= 'F') {
    lowByte -= 'A';
    lowByte += 0xA;
  } else if (lowByte >= 'a' && lowByte <= 'f') {
    lowByte -= 'a';
    lowByte += 0xA;
  } else {
    return -1;
  }
  return ((highByte & 0xF) << 4) + (lowByte & 0xF);
}

static bool getStringFromHex(String serial, int& index, String& result) {
  result = "";
  while (serial[index] != '/') {
    char character = x2c(serial[index++], serial[index++]);
    if (character == -1) {
      println_dbg("Unserializer: character error");
      return false;
    }
    result += character;
  }
  index++;
  return true;
}

bool IRKit::unserializer(String serial) {
  // [0248]/#{SSID}/#{Password}/#{Key}/#{RegDomain}//////#{CRC}

  int index = 0;
  switch (serial[index++]) {
    case '0':
      security = 0;
      break;
    case '2':
      security = 2;
      break;
    case '4':
      security = 4;
      break;
    case '8':
      security = 8;
      break;
    default:
      println_dbg("Unserializer: security type error");
      return false;
  }

  if (serial[index++] != '/')return false;

  if (getStringFromHex(serial, index, ssid) == false)return false;
  if (getStringFromHex(serial, index, password) == false)return false;
  int start = index;
  int end = serial.indexOf('/', start);
  devicekey = serial.substring(start, end);
  index = end + 1;

  int regdomain = serial[index++] - '0';
  if (serial[index++] != '/')return false;

  String reserved2, reserved3, reserved4, reserved5, reserved6;
  if (getStringFromHex(serial, index, reserved2) == false)return false;
  if (getStringFromHex(serial, index, reserved3) == false)return false;
  if (getStringFromHex(serial, index, reserved4) == false)return false;
  if (getStringFromHex(serial, index, reserved5) == false)return false;
  if (getStringFromHex(serial, index, reserved6) == false)return false;

  println_dbg("SSID: " + ssid);
  println_dbg("PASSWORD: " + password);
}

String IRKit::settingsCrcSerial(void) {
  return String(mode, DEC) + ssid + password + devicekey + clienttoken;
}

bool IRKit::settingsRestoreFromFile() {
  String s;
  if (getStringFromFile(SETTINGS_DATA_PATH, s) == false) return false;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(s);
  mode = (int)data["mode"];
  ssid = (const char*)data["ssid"];
  password = (const char*)data["password"];
  devicekey = (const char*)data["devicekey"];
  clienttoken = (const char*)data["clienttoken"];
  uint8_t crc = (uint8_t)data["crc"];
  String serial = settingsCrcSerial();
  if (crc != crc8((uint8_t*)serial.c_str(), serial.length(), CRC8INIT)) {
    println_dbg("CRC8 difference");
    return false;
  }
  println_dbg("CRC8 OK");
  return true;
}

bool IRKit::settingsBackupToFile() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["mode"] = mode;
  data["ssid"] = ssid;
  data["password"] = password;
  data["devicekey"] = devicekey;
  data["clienttoken"] = clienttoken;
  String serial = settingsCrcSerial();
  data["crc"] = crc8((uint8_t*)serial.c_str(), serial.length(), CRC8INIT);
  String str;
  data.printTo(str);
  return writeStringToFile(SETTINGS_DATA_PATH, str);
}

