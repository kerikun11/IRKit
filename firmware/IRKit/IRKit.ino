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
#include "httpsClient.h"
#include "setup.h"

void setup() {
  // Prepare Serial debug
  DEBUG_SERIAL.begin(DEBUG_SERIAL_BAUDRATE);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare GPIO
  pinMode(PIN_INDICATE_LED, OUTPUT);
  pinMode(PIN_IR_IN, INPUT);
  pinMode(PIN_IR_OUT, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // Setup Start
  digitalWrite(PIN_INDICATE_LED, HIGH);

  // OTA setup
  setupOTA();

  // Mode setup
  irkit.setup();

  // Setup Completed
  digitalWrite(PIN_INDICATE_LED, LOW);
  println_dbg("Setup Completed");
}

void buttonTask() {
  /* disconnect wifi by SW */
  static uint32_t timeStamp;
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (millis() - timeStamp > 2000) {
      timeStamp = millis();
      println_dbg("Button long pressed");
      irkit.setMode(IRKIT_MODE_NULL);
      ESP.reset();
    }
  } else {
    timeStamp = millis();
  }
}

void loop() {
  OTATask();
  serverTask();
  if (irkit.mode == IRKIT_MODE_STA) {
    clientTask();
    irTask();
  }
}

