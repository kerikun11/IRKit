/*
   IR-station Ver.1.0.0
   Infrared Remote Controller with ESP8266 WiFi Module

   Author:  kerikun11 (Github: kerikun11)
   Date:    2016.01.22
*/

#include <ESP8266WiFi.h>
#include "config.h"
#include "wifi.h"
#include "ota.h"
#include "httpServer.h"
#include "setup.h"

void setup() {
  // Prepare Serial debug
  Serial.begin(115200);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare GPIO
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_IR_IN, INPUT);
  pinMode(PIN_IR_OUT, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);

  // Setup Start
  digitalWrite(PIN_LED1, HIGH);

  // OTA setup
  setupOTA();

  // irkit.setMode(IR_STATION_MODE_NULL);

  // Mode setup
  irkit.setup();

  // Setup Completed
  digitalWrite(PIN_LED1, LOW);
  println_dbg("Setup Completed");
}

void loop() {
  OTATask();
  serverTask();
  irTask();
  /* disconnect wifi by SW */
  static uint32_t timeStamp;
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (millis() - timeStamp > 2000) {
      timeStamp = millis();
      println_dbg("Button long pressed");
//      connectWifi("WiFi-2.4GHz", "kashimamerda"); /*< for develop */
      irkit.setMode(IR_STATION_MODE_NULL);
      //      ESP.reset();
    }
  } else {
    timeStamp = millis();
  }
}

