/*
   IRKit with ESP-WROOM-02

   Author:  kerikun11 (Github: kerikun11)
   Date:    2016.01.22
*/

#include <ESP8266WiFi.h>
#include "config.h"
#include "ota.h"
#include "httpServer.h"
#include "httpsClient.h"
#include "setup.h"

void setup() {
  // Prepare Serial debug
  DEBUG_SERIAL.begin(DEBUG_SERIAL_BAUDRATE);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02.");

  // prepare GPIO
  pinMode(PIN_INDICATE_LED, OUTPUT);
  pinMode(PIN_IR_IN, INPUT);
  pinMode(PIN_IR_OUT, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // Setup Start
  digitalWrite(PIN_INDICATE_LED, HIGH);

  // Mode setup
  irkit.setup();

  // Setup Completed
  digitalWrite(PIN_INDICATE_LED, LOW);
  println_dbg("Setup Completed");
}

void loop() {
#if USE_OTA_UPDATE == true
  OTATask();
#endif
  serverTask();
  if (irkit.mode == IRKIT_MODE_STA) {
    clientTask();
    irTask();
  }
}

