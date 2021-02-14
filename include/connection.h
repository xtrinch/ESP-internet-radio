
#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "common.h"
#include <WebServer.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include "config.h"

#ifdef MQTT_ENABLED
#include "connection-mqtt.h"
#endif

#define CFG_WIFI_SSID xstr(WIFI_SSID)
#define CFG_WIFI_PASSWORD xstr(WIFI_PASSWORD)

void connectionSetup();

#endif