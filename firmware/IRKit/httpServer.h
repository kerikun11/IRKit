#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include <ESP8266WiFi.h>

void serverTask();

void setupServer(void);
void setupAPServer(void);

#endif

