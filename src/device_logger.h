#ifndef DEVICE_LOGGER_H
#define DEVICE_LOGGER_H

#include "websocket_interface.h"
#include <ESP8266WiFi.h>

struct Device {
  String mac;
  IPAddress ip;
  String name;
};

struct DeviceLog {
  String mac;
  IPAddress ip;
  unsigned long timestamp;
  bool connected;
  String name;
};

extern std::vector<DeviceLog> connectionLog;

void checkConnectedDevices();
String getLogPre();

#endif
