
#ifndef __MQTT_CONNECTION_H__
#define __MQTT_CONNECTION_H__

#include "common.h"
#include <WebServer.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include "config.h"

#define CFG_MQTT_UPSTREAM_TOPIC xstr(MQTT_UPSTREAM_TOPIC)
#define CFG_MQTT_DOWNSTREAM_TOPIC xstr(MQTT_DOWNSTREAM_TOPIC)
#define CFG_MQTT_HOST xstr(MQTT_HOST)

extern TimerHandle_t mqttReconnectTimer;

void mqttSetup();
void connectToMqtt();

#endif