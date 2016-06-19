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
      digitalWrite(PIN_LED1, HIGH);
      break;
    case IR_RECEIVER_RECEIVING:
      while (diff > 0xFFFF) {
        if (signal.rawIndex > RAWDATA_BUFFER_SIZE - 2) {
          println_dbg("IR buffer overflow");
          break;
        }
        signal.rawData[signal.rawIndex++] = 0xFFFF;
        signal.rawData[signal.rawIndex++] = 0;
        diff -= 0xFFFF;
      }
      if (signal.rawIndex > RAWDATA_BUFFER_SIZE - 1) {
        println_dbg("IR buffer overflow");
        break;
      }
      signal.rawData[signal.rawIndex++] = diff;
      break;
    case IR_RECEIVER_READING:
      break;
  }

  signal.prev_us = us;
}

void irTask() {
  noInterrupts();
  uint32_t diff = micros() - signal.prev_us;
  interrupts();

  switch (signal.state) {
    case IR_RECEIVER_READY:
      break;
    case IR_RECEIVER_RECEIVING:
      if (diff > IR_RECEIVE_TIMEOUT_US) {
        signal.state = IR_RECEIVER_READING;
        println_dbg("End Receiving");
        digitalWrite(PIN_LED1, LOW);
      }
      break;
    case IR_RECEIVER_READING:
      if (signal.rawIndex < 10) {
        println_dbg("noise");
        signal.state = IR_RECEIVER_READY;
        break;
      }

      println_dbg("Raw Data: ");
      for (int i = 0; i < signal.rawIndex; i++) {
        print_dbg(signal.rawData[i], DEC);
        if (i != signal.rawIndex - 1) print_dbg(",");
      }
      println_dbg("");

      volatile uint8_t buff[RAWDATA_BUFFER_SIZE];
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

      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["format"] = "raw";
      root["freq"] = 38;
      JsonArray& data = root.createNestedArray("data");
      for (int i = 0; i < signal.rawIndex; i++) {
        data.add(signal.rawData[i]);
      }
      root.printTo(signal.irJson);

      signal.state = IR_RECEIVER_READY;
      break;
  }
}

void IR_SIGNAL::send(String dataJson) {
  //  println_dbg(dataJson);
  digitalWrite(PIN_LED1, HIGH);
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
        digitalWrite(PIN_IR_OUT, !(count & 1));
        delayMicroseconds(8);
        digitalWrite(PIN_IR_OUT, 0);
        delayMicroseconds(16);
      } while (int32_t(us + time - micros()) > 0);
    }
    digitalWrite(PIN_LED1, LOW);
  }
  interrupts();
  println_dbg("Send OK");
}

