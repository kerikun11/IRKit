#include "irSignal.h"

#include <ArduinoJson.h>
#include "config.h"
#include "IrPacker.h"
#include "base64encoder.h"
#include "httpsClient.h"
#include "setup.h"

class IR_SIGNAL signal;

void irExternalISR() {
  uint32_t us = micros();
  uint32_t diff = (us - signal.prev_us) * 2;

  switch (signal.state) {
    case IR_RECEIVER_READY:
      signal.state = IR_RECEIVER_RECEIVING;
      signal.rawIndex = 0;
      break;
    case IR_RECEIVER_RECEIVING:
      while (diff > 0xFFFF) {
        if (signal.rawIndex > RAWDATA_BUFFER_SIZE - 1)break;
        signal.rawData[signal.rawIndex++] = 0xFFFF;
        signal.rawData[signal.rawIndex++] = 0;
        diff -= 0xFFFF;
      }
      if (signal.rawIndex > RAWDATA_BUFFER_SIZE - 1)break;
      signal.rawData[signal.rawIndex++] = diff;
      break;
    case IR_RECEIVER_READING:
      break;
  }

  signal.prev_us = us;
}

void irTask() {
  uint32_t us = micros();

  switch (signal.state) {
    case IR_RECEIVER_READY:
      break;
    case IR_RECEIVER_RECEIVING:
      if (us - signal.prev_us > IR_RECEIVE_TIMEOUT_US) {
        signal.state = IR_RECEIVER_READING;
        if (signal.rawIndex < 10) {
          signal.state = IR_RECEIVER_READY;
          break;
        }
        println_dbg("End Receiving");

        println_dbg("Raw Data: ");
        for (int i = 0; i < signal.rawIndex; i++) {
          print_dbg(signal.rawData[i], DEC);
          if (i != signal.rawIndex - 1) print_dbg(",");
        }
        println_dbg("");

        volatile uint8_t buff[200];
        volatile struct irpacker_t packer_state;
        irpacker_init(&packer_state, buff);
        for (int i = 0; i < signal.rawIndex; i++) {
          irpacker_pack(&packer_state, signal.rawData[i]);
        }
        irpacker_packend(&packer_state);
        int length = irpacker_length(&packer_state);

        signal.encoded = "";
        base64_encode((const uint8_t*)buff, length, [](char encode) {
          signal.encoded += (char)encode;
        });
        println_dbg("");
        String res = httpPost("/p?devicekey=" + irkit.devicekey + "&freq=38", signal.encoded, 1000);
        signal.state = IR_RECEIVER_READY;
      }
      break;
    case IR_RECEIVER_READING:
      break;
  }
}

void IR_SIGNAL::send(String dataJson) {
  println_dbg(dataJson);
  digitalWrite(PIN_LED1, HIGH);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(dataJson);
  analogWriteFreq((int)root["freq"] * 1000);
  for (uint16_t count = 0; count < root["data"].size(); count++) {
    wdt_reset();
    uint32_t us = micros();
    uint16_t time = (uint16_t)root["data"][count];
    time /= 2;
    if (time != 0) {
      if (!(count & 1))analogWrite(PIN_IR_OUT, 300);
      else analogWrite(PIN_IR_OUT, 0);
    }
    while (int32_t(us + time - micros()) > 0)wdt_reset();
  }
  analogWrite(PIN_IR_OUT, 0);
  digitalWrite(PIN_LED1, LOW);
  println_dbg("Send OK");
}

