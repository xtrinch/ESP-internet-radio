
#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "common.h"
#include <WebServer.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <TridentTD_ESP32NVS.h>
#include "stream-mp3.h"

#define CFG_WIFI_SSID xstr(WIFI_SSID)
#define CFG_WIFI_PASSWORD xstr(WIFI_PASSWORD)

#define CFG_MQTT_UPSTREAM_TOPIC xstr(MQTT_UPSTREAM_TOPIC)
#define CFG_MQTT_DOWNSTREAM_TOPIC xstr(MQTT_DOWNSTREAM_TOPIC)
#define CFG_MQTT_HOST xstr(MQTT_HOST)

#define NVS_CONFIG_KEY "configz"

void connectionSetup();

#endif