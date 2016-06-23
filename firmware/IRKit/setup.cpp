#include "setup.h"

#include <ArduinoJson.h>
#include "config.h"
#include "httpServer.h"
#include "wifi.h"
#include "ota.h"
#include "file.h"
#include "CRC8.h"

class IRKit irkit;

void IRKit::setup(void) {
  wdt_reset();

  generateHostname();

  setupFile();
  settingsRestoreFromFile();

  switch (mode) {
    case IRKIT_MODE_NULL:
      println_dbg("Boot Mode: NULL");
      configureAccesspoint(hostname, "");
      setupAPServer();
      break;
    case IRKIT_MODE_STA:
      println_dbg("Boot Mode: Station");
      connectCachedWifi();
      attachInterrupt(PIN_IR_IN, irExternalISR, CHANGE);
      setupServer();
      break;
  }
}

void IRKit::setMode(uint8_t newMode) {
  mode = newMode;
  settingsBackupToFile();
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

char x2c(char highByte, char lowByte) {
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

bool getStringFromHex(String serial, int& index, String& result) {
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
  enum UNSERIALIZE_STATE {
    UNSERIALIZE_SECURITY,
    UNSERIALIZE_SSID,
    UNSERIALIZE_PASSWORD,
    UNSERIALIZE_KEY,
    UNSERIALIZE_REGDOMAIN,
    UNSERIALIZE_RESERVED2,
    UNSERIALIZE_RESERVED3,
    UNSERIALIZE_RESERVED4,
    UNSERIALIZE_RESERVED5,
    UNSERIALIZE_RESERVED6,
    UNSERIALIZE_CRC,
  } state = UNSERIALIZE_SECURITY;

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

  uint8_t crc = x2c(serial[index++], serial[index++]);
  println_dbg("CRC: " + String(crc, DEC));
  if (crc == -1) {
    println_dbg("Unserializer: crc character error");
    return false;
  }
  uint8_t generatedCrc = crc8((uint8_t*)serial.c_str(), index - 2, CRC8INIT);
  println_dbg("generated CRC: " + String(generatedCrc, DEC));

  println_dbg("Security: " + String(security, DEC));
  println_dbg("SSID: " + ssid);
  println_dbg("PASSWORD: " + password);
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

