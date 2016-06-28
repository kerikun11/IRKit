#ifndef setup_op_h
#define setup_op_h

#include <ESP8266WiFi.h>
#include "irSignal.h"

#define IRKIT_MODE_NULL  0
#define IRKIT_MODE_STA   1


class IRKit {
  public:
    uint8_t mode;

    String hostname;
    String version;
    String devicekey;
    String clienttoken;

    uint8_t security;
    String ssid;
    String password;

    void setup();
    void reset();
    void setMode(uint8_t newMode);
    bool unserializer(String serial);

  private:
    void setupButtonInterrupt();
    void generateHostname();

    String settingsCrcSerial(void);
    bool settingsRestoreFromFile();
    bool settingsBackupToFile();
};

extern class IRKit irkit;

#endif

