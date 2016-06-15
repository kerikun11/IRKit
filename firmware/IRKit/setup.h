#ifndef setup_op_h
#define setup_op_h

#include <ESP8266WiFi.h>
#include "irSignal.h"

#define IR_STATION_MODE_NULL  0
#define IR_STATION_MODE_STA   1


class IRKit {
  public:
    uint8_t mode = IR_STATION_MODE_NULL;
    String devicekey;
    String clienttoken;

    String hostname;
    String version;

    uint8_t security;
    String ssid;
    String password;

    void setup();
    void setMode(uint8_t newMode);
    bool unserializer(String serial);

  private:
    void generateHostname();

    void settingsRestoreFromFile();
    void settingsBackupToFile();
};

extern class IRKit irkit;

#endif

