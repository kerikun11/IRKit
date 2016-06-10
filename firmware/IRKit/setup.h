#ifndef setup_op_h
#define setup_op_h

#include <ESP8266WiFi.h>
#include "irSignal.h"

#define IR_STATION_MODE_NULL  0
#define IR_STATION_MODE_STA   1


class IRKit {
  public:
    uint8_t mode = IR_STATION_MODE_STA;
    String devicekey;
    String clienttoken;
    String hostname = "irkit1234";
    String version = "1.0.0";

    void setup();
    void setMode(uint8_t newMode);
    bool serialParser(String serial);
    void settingsRestoreFromFile();
    void settingsBackupToFile();

  private:

};

extern class IRKit irkit;

#endif

