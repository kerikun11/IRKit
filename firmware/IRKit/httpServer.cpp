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

void setupServer(void) {
  server.on("/messages", HTTP_GET, []() {
    dispRequest();
    server.send(200, "text/plain", signal.irJson);
    signal.irJson = "";
    println_dbg("End");
  });
  server.on("/messages", HTTP_POST, []() {
    dispRequest();
    String req = server.arg(0);
    if ((req.indexOf("{") >= 0) && (req.indexOf("}") >= 0)) {
      signal.send(req);
      server.send(200);
    } else {
      server.send(400);
    }
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

void setupAPServer(void) {
  server.on("/", HTTP_GET, []() {
    dispRequest();
    WiFiClient client = server.client();
    client.println("HTTP/1.0 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Server: IRKit/" + irkit.version);
    client.println("Content-Type: text/plain");
    client.println("");
    println_dbg("End");
  });
  server.on("/wifi", HTTP_POST, []() {
    dispRequest();
    irkit.unserializer(server.arg(0));
    server.send(200);

    if (connectWifi(irkit.ssid, irkit.password)) {
      println_dbg("connection successful");

      String res;
      res = httpPost("/d", "devicekey=" + irkit.devicekey + "&hostname=" + irkit.hostname, 3000);
      if (res.indexOf("200 OK") < 0) {
        println_dbg("End");
        server.send(500);
        return;
      }
      res = httpPost("/k", "devicekey=" + irkit.devicekey, 3000);
      if (res.indexOf("200 OK") < 0) {
        println_dbg("End");
        server.send(500);
        return;
      }
      res = res.substring(res.indexOf("\r\n\r\n"));
      if ((res.indexOf("{") >= 0) && (res.indexOf("}") >= 0)) {
        print_dbg("Json: ");
        println_dbg(res);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(res);
        irkit.clienttoken = (const char*)root["clienttoken"];
        println_dbg("clinettoken: " + irkit.clienttoken);

        irkit.setMode(IRKIT_MODE_STA);
        delay(100);
        println_dbg("End");
        ESP.reset();
      }
    } else {
      println_dbg("connection failed");
      println_dbg("End");
    }
  });
  server.onNotFound([]() {
    dispRequest();
    println_dbg("file not found");
    server.send(404, "text/plain", "FileNotFound");
    println_dbg("End");
  });

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("AP Server Listening");
}

