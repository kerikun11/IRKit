#include "httpsClient.h"

#include <ArduinoJson.h>
#include "config.h"
#include "setup.h"

void clientTask() {
  static int newer_than = 0;
  static bool keep_alive = false;
  static WiFiClient client;
  
  if (keep_alive == false) {
    if (!client.connect(IRKIT_SERVER_HOST, IRKIT_SERVER_PORT)) {
      println_dbg("connection failed");
      return;
    }
    println_dbg("GET /m?devicekey=" + irkit.devicekey + "&newer_than=" + String(newer_than, DEC) + " HTTP/1.1");
    println_dbg("User-Agent: IRKit/1.0.0");
    println_dbg("Host: "IRKIT_SERVER_HOST);
    println_dbg("");

    client.println("GET /m?devicekey=" + irkit.devicekey + "&newer_than=" + String(newer_than, DEC) + " HTTP/1.1");
    client.println("User-Agent: IRKit/1.0.0");
    client.println("Host: "IRKIT_SERVER_HOST);
    client.println("");

    keep_alive = true;
  } else {
    if (client.available()) {
      client.setTimeout(10);
      String res = client.readString();
      client.stop();
      println_dbg("Response:");
      println_dbg(res);
      if (res.indexOf("{") > 0 && res.indexOf("}") > 0) {
        res = res.substring(res.indexOf("\r\n\r\n"));
        signal.state = IR_RECEIVER_OFF;
        signal.send(res);
        signal.state = IR_RECEIVER_READY;
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(res);
        newer_than = (int)root["id"];
      }
      keep_alive = false;
    }
  }
}

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

