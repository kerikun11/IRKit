  /*
   OTA operation

   void setup(){
     ...
     setupOTA();
     ...
   }
   void loop(){
     ...
     OTATask();
     ...
   }
*/
#ifndef __OTA_H__
#define __OTA_H__

#include <ESP8266WiFi.h>

void setupOTA();
void OTATask();

#endif

