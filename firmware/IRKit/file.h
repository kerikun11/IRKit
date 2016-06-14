#ifndef file_h
#define file_h

#include <ESP8266WiFi.h>

void setupFile();

bool writeStringToFile(String path, String dataString);
bool getStringFromFile(String path, String& dataString);

#endif

