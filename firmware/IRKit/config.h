#ifndef __CONFIG_H__
#define __CONFIG_H__

/* IRKit device version */
#define IRKIT_VERSION           "3.0.0-esp8266"

/* Hardware Mapping */
#define PIN_BUTTON              0
#define PIN_IR_IN               5
#define PIN_IR_OUT              14
#define PIN_INDICATE_LED        13

/* WiFi settings */
#define WIFI_CONNECT_TIMEOUT    10 // unit: second
#define WIFI_AP_PASSWORD        "XXXXXXXX"

/* SPIFFS saving path */
#define SETTINGS_DATA_PATH      "/settings.json"

/* IRKit server info */
#define IRKIT_SERVER_HOST       "api.getirkit.com"
#define IRKIT_SERVER_PORT       80

/* OTA Update */
#define USE_OTA_UPDATE          true

/* Debug Settings */
#define DEBUG_PRINT_INFO        true     /*< デバック出力有無 */
#define DEBUG_SERIAL            Serial  /*< デバック用シリアル */
#define DEBUG_SERIAL_BAUDRATE   115200

#if DEBUG_PRINT_INFO == true
# define print_dbg              DEBUG_SERIAL.print
# define printf_dbg             DEBUG_SERIAL.printf
# define println_dbg            DEBUG_SERIAL.println
#else
# define print_dbg              // No Operation
# define printf_dbg             // No Operation
# define println_dbg            // No Operation
#endif

#endif

