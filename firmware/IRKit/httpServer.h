#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include <ESP8266WiFi.h>

void dispRequest();
void serverTask();

//String extract(String target, String head, String tail);

void setupServer(void);

#endif

