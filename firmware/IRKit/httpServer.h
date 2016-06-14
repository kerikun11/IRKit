#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include <ESP8266WiFi.h>

void serverTask();
void dispRequest();

void setupServer(void);

#endif

