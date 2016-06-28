#ifndef __IR_SIGNAL__
#define __IR_SIGNAL__

#include <ESP8266WiFi.h>

#define IR_RECEIVE_TIMEOUT_US   200000

#define RAWDATA_BUFFER_SIZE     800

enum IR_RECEIVER_STATE {
  IR_RECEIVER_OFF,
  IR_RECEIVER_READY,
  IR_RECEIVER_RECEIVING,
  IR_RECEIVER_READING,
};

class IR_SIGNAL {
  public:
    IR_SIGNAL(int txPin, int rxPin);
    volatile enum IR_RECEIVER_STATE state = IR_RECEIVER_OFF;

    volatile uint16_t rawIndex;
    volatile uint16_t rawData[RAWDATA_BUFFER_SIZE];
    volatile uint32_t prev_us = 0;
    String encoded;
    String irJson;

    void setState(enum IR_RECEIVER_STATE newState);
    void send(String dataJson);
  private:
    void irExternalISR();
};

void irTask();


#endif

