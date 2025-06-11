#include "device_logger.h"

std::vector<DeviceLog> connectionLog;

static std::vector<Device> previousDevices;

void checkConnectedDevices() {
  std::vector<Device> currentDevices;

  struct station_info *station_list = wifi_softap_get_station_info();
  while (station_list != NULL) {
    IPAddress ip(station_list->ip.addr);
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            station_list->bssid[0], station_list->bssid[1], station_list->bssid[2],
            station_list->bssid[3], station_list->bssid[4], station_list->bssid[5]);

    currentDevices.push_back({String(macStr), ip});

    station_list = STAILQ_NEXT(station_list, next);
  }
  wifi_softap_free_station_info();

  unsigned long now = millis();

  for (const auto &device : currentDevices) {
    bool found = false;
    for (const auto &prev : previousDevices) {
      if (device.mac == prev.mac) {
        found = true;
        break;
      }
    }
    if (!found) {
      connectionLog.push_back({device.mac, device.ip, now, true, device.name});
      String mac = device.mac;
      String ip = device.ip.toString();
      Serial.printf("[+] Conectado: %s (%s)\n", mac.c_str(), ip.c_str());
      String data = "{\"Mac\":\"" + mac + "\",\"IP\":\"" + ip + "\"}";
      ws.textAll("{\"type\": \"connect\", \"data\": " + data + "}");
    }
  }

  for (const auto &prev : previousDevices) {
    bool stillConnected = false;
    for (const auto &device : currentDevices) {
      if (device.mac == prev.mac) {
        stillConnected = true;
        break;
      }
    }
    if (!stillConnected) {
      connectionLog.push_back({prev.mac, prev.ip, now, false, prev.name});
      String mac = prev.mac;
      String ip = prev.ip.toString();
      Serial.printf("[-] Desconectado: %s (%s)\n", mac.c_str(), ip.c_str());
      String data = "{\"Mac\":\"" + mac + "\",\"IP\":\"" + ip + "\"}";
      ws.textAll("{\"type\": \"disconnect\", \"data\": " + data + "}");
    }
  }

  previousDevices = currentDevices;
}

String getLogPre() {
  String response = "<pre>";
  for (const auto &log : connectionLog) {
    response += log.connected ? "[+]" : "[-]";
    response += " " + log.mac + " (" + log.ip.toString() + ")";
    response += " at " + String(log.timestamp) + "ms";
    response += " as '" + log.name + "'\n";
  }
  response += "</pre>";
  response += R"rawliteral(
    <script>
    document.addEventListener("DOMContentLoaded", () => {
      const button = document.createElement("button");
      button.textContent = "Cargar Vendor JSON";
      button.style.marginBottom = "10px";

      const fileInput = document.createElement("input");
      fileInput.type = "file";
      fileInput.accept = "application/json";
      fileInput.style.display = "none";

      button.onclick = () => fileInput.click();

      fileInput.onchange = async () => {
        const file = fileInput.files[0];
        if (!file) return;

        const text = await file.text();
        let vendorData;

        try {
          vendorData = JSON.parse(text);
        } catch (err) {
          alert("JSON inválido.");
          return;
        }

        const prefixMap = new Map();
        vendorData.forEach(entry => {
          const normPrefix = entry.macPrefix.toUpperCase();
          prefixMap.set(normPrefix, entry.vendorName);
        });

        const pre = document.querySelector("pre");
        const lines = pre.textContent.split("\n");
        const updatedLines = lines.map(line => {
          const match = line.match(/([0-9A-F]{2}:[0-9A-F]{2}:[0-9A-F]{2})/i);
          if (!match) return line;

          const macPrefix = match[1].toUpperCase();
          const vendor = prefixMap.get(macPrefix);
          if (vendor) {
            return line.replace(/as\s+''/, `as '${vendor}'`);
          }
          return line;
        });

        pre.textContent = updatedLines.join("\n");
      };

      document.body.insertBefore(button, document.body.firstChild);
      document.body.insertBefore(fileInput, button.nextSibling);
    });
    </script>
  )rawliteral";
  return response;
}
