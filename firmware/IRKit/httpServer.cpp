#include "httpServer.h"

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "config.h"
#include "httpsClient.h"
#include "irSignal.h"
#include "wifi.h"
#include "setup.h"

// TCP server at port 80 will respond to HTTP requests
static ESP8266WebServer server(80);

void serverTask() {
  server.handleClient();
}

void dispRequest() {
  println_dbg("");
  println_dbg("New Request URI: " + server.uri() + " Method: " + ((server.method() == HTTP_GET) ? "GET" : "POST"));
  println_dbg("Arguments: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("  " + server.argName(i) + " = " + server.arg(i));
  }
}

//String extract(String target, String head, String tail) {
//  return target.substring(target.indexOf(head) + head.length(), target.indexOf(tail, target.indexOf(head) + head.length()));
//}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void setupServer(void) {
  server.on("/", HTTP_GET, []() {
    dispRequest();
    WiFiClient client = server.client();
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Server: IRKit/" + irkit.version);
    client.println("Content-Type: text/plain");
    client.println("");
    println_dbg("End");
  });
  server.on("/messages", HTTP_GET, []() {
    dispRequest();
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["format"] = "raw";
    root["freq"] = 38;
    JsonArray& data = root.createNestedArray("data");
    for (int i = 0; i < 32; i++) {
      data.add(1234);
    }
    String res;
    root.printTo(res);
    server.send(200, "text/plain", res);
    println_dbg("End");
  });
  server.on("/messages", HTTP_POST, []() {
    dispRequest();
    signal.state = IR_RECEIVER_OFF;
    signal.send(server.arg(0));
    signal.state = IR_RECEIVER_READY;
    server.send(200);
    println_dbg("End");
  });
  server.on("/keys", HTTP_POST, []() {
    dispRequest();
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["clienttoken"] = irkit.clienttoken;
    String res;
    root.printTo(res);
    server.send(200, "text/plain", res);
    println_dbg("End");
  });
  server.on("/wifi", HTTP_POST, []() {
    dispRequest();
    irkit.unserializer(server.arg(0));
    if (connectWifi(irkit.ssid, irkit.password)) {
      println_dbg("connection successful");
      server.send(200);

      String res;
      res = httpPost("/d", "devicekey=" + irkit.devicekey + "&hostname=" + irkit.hostname, 1000);
      if (res.indexOf("200 OK") < 0) {
        server.send(400);
        println_dbg("End");
        return;
      }
      res = httpPost("/k", "devicekey=" + irkit.devicekey, 1000);
      if (res.indexOf("200 OK") < 0) {
        server.send(400);
        println_dbg("End");
        return;
      }
      res = res.substring(res.indexOf("\r\n\r\n"));
      print_dbg("Json: ");
      println_dbg(res);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(res);
      irkit.clienttoken = (const char*)root["clienttoken"];
      println_dbg("clinettoken: " + irkit.clienttoken);

      irkit.setMode(IR_STATION_MODE_STA);
      delay(100);
      println_dbg("End");
      ESP.reset();
    } else {
      println_dbg("connection failed");
      server.send(400);
      println_dbg("End");
    }
  });
  server.onNotFound([]() {
    dispRequest();
    println_dbg("file not found");
    server.send(404, "text/plain", "FileNotFound");
    println_dbg("End");
  });

  // Set up mDNS responder:
  print_dbg("mDNS address: ");
  println_dbg("http://" + irkit.hostname + ".local");
  if (!MDNS.begin(irkit.hostname.c_str())) {
    println_dbg("Error setting up MDNS responder!");
  } else {
    println_dbg("mDNS responder started");
  }
  MDNS.addService("irkit", "tcp", 80);

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("Server Listening");
}

