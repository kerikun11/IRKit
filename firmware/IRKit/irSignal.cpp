#include "irSignal.h"

#include <ArduinoJson.h>
#include <base64.h>
#include "config.h"
#include "IrPacker.h"
#include "httpsClient.h"
#include "setup.h"

IR_SIGNAL::IR_SIGNAL(uint8_t tx, uint8_t rx) {
  txPin = tx;
  rxPin = rx;
  pinMode(tx, OUTPUT);
  pinMode(rx, INPUT);
}

void IR_SIGNAL::setState(enum IR_RECEIVER_STATE newState) {
  state = newState;
}

void IR_SIGNAL::send(String dataJson) {
  state = IR_RECEIVER_OFF;
  {
    digitalWrite(PIN_INDICATE_LED, HIGH);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(dataJson);
    noInterrupts();
    {
      for (uint16_t count = 0; count < root["data"].size(); count++) {
        wdt_reset();
        uint32_t us = micros();
        uint16_t time = (uint16_t)root["data"][count];
        time /= 2;
        do {
          digitalWrite(txPin, !(count & 1));
          delayMicroseconds(8);
          digitalWrite(txPin, 0);
          delayMicroseconds(16);
        } while (int32_t(us + time - micros()) > 0);
      }
      digitalWrite(PIN_INDICATE_LED, LOW);
    }
    interrupts();
  }
  state = IR_RECEIVER_READY;
  println_dbg("Send OK");
}

void IR_SIGNAL::isr() {
  uint32_t us = micros();
  uint32_t diff = (us - prev_us) * 2;

  switch (state) {
    case IR_RECEIVER_READY:
      state = IR_RECEIVER_RECEIVING;
      rawIndex = 0;
      digitalWrite(PIN_INDICATE_LED, HIGH);
      break;
    case IR_RECEIVER_RECEIVING:
      while (diff > 0xFFFF) {
        if (rawIndex > RAWDATA_BUFFER_SIZE - 2) {
          println_dbg("IR buffer overflow");
          break;
        }
        rawData[rawIndex++] = 0xFFFF;
        rawData[rawIndex++] = 0;
        diff -= 0xFFFF;
      }
      if (rawIndex > RAWDATA_BUFFER_SIZE - 1) {
        println_dbg("IR buffer overflow");
        break;
      }
      rawData[rawIndex++] = diff;
      break;
    case IR_RECEIVER_READING:
      break;
  }

  prev_us = us;
}

void IR_SIGNAL::handle() {
  noInterrupts();
  uint32_t diff = micros() - prev_us;
  interrupts();

  switch (state) {
    case IR_RECEIVER_READY:
      break;
    case IR_RECEIVER_RECEIVING:
      if (diff > IR_RECEIVE_TIMEOUT_US) {
        state = IR_RECEIVER_READING;
        println_dbg("End Receiving");
        digitalWrite(PIN_INDICATE_LED, LOW);
      }
      break;
    case IR_RECEIVER_READING:
      if (rawIndex < 8) {
        println_dbg("noise");
        state = IR_RECEIVER_READY;
        break;
      }

      println_dbg("Raw Data: ");
      for (int i = 0; i < rawIndex; i++) {
        print_dbg(rawData[i], DEC);
        if (i != rawIndex - 1) print_dbg(",");
      }
      println_dbg("");

      uint8_t buff[RAWDATA_BUFFER_SIZE];
      struct irpacker_t packer_state;
      irpacker_init(&packer_state, buff);
      for (int i = 0; i < rawIndex; i++) {
        irpacker_pack(&packer_state, rawData[i]);
      }
      irpacker_packend(&packer_state);

      base64 binary;
      String encoded = binary.encode((uint8_t*)buff, irpacker_length(&packer_state));

      println_dbg("");
      String res = httpPost("/p?devicekey=" + irkit.devicekey + "&freq=38", encoded, 1000);

      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["format"] = "raw";
      root["freq"] = 38;
      JsonArray& data = root.createNestedArray("data");
      for (int i = 0; i < rawIndex; i++) {
        data.add(rawData[i]);
      }
      root.printTo(irJson);

      state = IR_RECEIVER_READY;
      break;
  }
}

