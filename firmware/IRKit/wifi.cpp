#include "wifi.h" 

#include "config.h"

void configureAccesspoint(String ssid, String password) {
  wdt_reset();
  println_dbg("Configuring Access Point...");
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid.c_str(), password.c_str());

  // display information
  print_dbg("AP SSID : ");
  println_dbg(ssid);
  print_dbg("AP Password : ");
  println_dbg(password);
  print_dbg("AP IP address: ");
  println_dbg(WiFi.softAPIP());
}

bool connectWifi(String target_ssid, String target_pass) {
  wdt_reset();
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    println_dbg("SSID: " + String(WiFi.SSID(i)));
    if (target_ssid == String(WiFi.SSID(i))) {
      break;
    }
    if (i == n - 1) {
      println_dbg("");
      print_dbg("Couldn't find SSID: ");
      println_dbg(target_ssid);
      return false;
    }
  }
  println_dbg("");
  print_dbg("Connecting to SSID: ");
  println_dbg(target_ssid);
  WiFi.begin(target_ssid.c_str(), target_pass.c_str());

  // Wait for connection
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    ESP.wdtFeed();
    delay(500);
    print_dbg(".");
    timeout++;
    if (timeout >= 2 * WIFI_CONNECT_TIMEOUT) {
      println_dbg("");
      println_dbg("Invalid SSID or Password");
      println_dbg("WiFi Connection Failed");
      return false;
    }
  }
  println_dbg("");
  print_dbg("Connected to ");
  println_dbg(target_ssid);
  print_dbg("IP address: ");
  println_dbg(WiFi.localIP());

  return true;
}

