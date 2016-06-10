#include "httpsClient.h"

#include <ArduinoJson.h>
#include "config.h"

String httpPost(String path, String body, uint16_t timeout) {
  print_dbg("connecting to ");
  println_dbg(IRKIT_SERVER_HOST);
  WiFiClient client;
  if (!client.connect(IRKIT_SERVER_HOST, IRKIT_SERVER_PORT)) {
    println_dbg("connection failed");
    return "";
  }
  print_dbg("Path: ");
  println_dbg(path);
  print_dbg("Body: ");
  println_dbg(body);

  println_dbg("POST " + path + " HTTP/1.1");
  println_dbg("User-Agent: IRKit/"IRKIT_VERSION);
  println_dbg("Host: "IRKIT_SERVER_HOST);
  println_dbg("Content-Length: " + String(body.length(), DEC));
  println_dbg("Content-Type: application/x-www-form-urlencoded");
  println_dbg("");
  println_dbg(body);

  client.println("POST " + path + " HTTP/1.1");
  client.println("User-Agent: IRKit/1.0.0");
  client.println("Host: "IRKIT_SERVER_HOST);
  client.println("Content-Length: " + String(body.length(), DEC));
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println("");
  client.println(body);

  uint32_t timestamp = millis();
  while (true) {
    wdt_reset();

    if (client.available()) break;

    if (millis() - timestamp > timeout) {
      println_dbg("Timeout: Read Server");
      return "";
    }
  }
  client.setTimeout(10);
  String resp = client.readString();
  println_dbg("Response: ");
  println_dbg(resp);
  client.stop();

  return resp;
}

