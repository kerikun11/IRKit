#ifndef __WIFI__
#define __WIFI__

#include <ESP8266WiFi.h>

void configureAccesspoint(String ssid, String password);

bool connectCachedWifi();
bool connectWifi(String ssid,String password);

#endif

