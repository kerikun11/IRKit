#ifndef __CONFIG_H__
#define __CONFIG_H__

/* IRKit device version */
#define IRKIT_VERSION           "3.0.0-esp8266"

/* Hardware Mapping */
#define PIN_BUTTON              0
#define PIN_IR_IN               5
#define PIN_IR_OUT              14
#define PIN_LED1                16

/* WiFi settings */
#define WIFI_CONNECT_TIMEOUT    20 // unit: second

/* SPIFFS saving path */
#define SETTINGS_DATA_PATH      "/settings.json"

/* IRKit server info */
#define IRKIT_SERVER_HOST       "api.getirkit.com"
#define IRKIT_SERVER_PORT       80

/* OTA Update */
#define USE_OTA_UPDATE          true

/* for Debug */
#define SERIAL_DEBUG            true

#if SERIAL_DEBUG == true
#define print_dbg               Serial.print
#define printf_dbg              Serial.printf
#define println_dbg             Serial.println
#else
#define print_dbg               // No Operation
#define printf_dbg              // No Operation
#define println_dbg             // No Operation
#endif

#endif

