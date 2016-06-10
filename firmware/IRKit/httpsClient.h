#ifndef __HTTPS_CLIENT_H__
#define __HTTPS_CLIENT_H__

#include <ESP8266WiFi.h>

String httpPost(String path, String body , uint16_t timeout);

#endif

