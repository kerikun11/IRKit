#include "wifi.h" 

#include "config.h"

void configureAccesspoint(String ssid, String password) {
  wdt_reset();
  WiFi.mode(WIFI_AP_STA);
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

bool connectCachedWifi() {
  wdt_reset();
  // set WiFi Mode
  WiFi.mode(WIFI_STA);
  // get cached SSID and Password
  String target_ssid = WiFi.SSID();
  String target_pass = WiFi.psk();
  print_dbg("Connecting to cached WiFi: ");
  println_dbg(target_ssid);
  // Connect to WiFi network
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

bool connectWifi(String ssid, String password) {
  wdt_reset();
  // set WiFi Mode
  WiFi.mode(WIFI_AP_STA);
  // Connect to WiFi network
  println_dbg("");
  print_dbg("Connecting to WiFi: ");
  println_dbg(ssid);
  print_dbg("Password: ");
  println_dbg(password);
  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait for connection
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    wdt_reset();
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
  println_dbg(ssid);
  print_dbg("IP address: ");
  println_dbg(WiFi.localIP());

  return true;
}

