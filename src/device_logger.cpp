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
      Serial.printf("[+] Conectado: %s (%s)\n", device.mac.c_str(), device.ip.toString().c_str());
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
      Serial.printf("[-] Desconectado: %s (%s)\n", prev.mac.c_str(), prev.ip.toString().c_str());
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
  return response;
}
