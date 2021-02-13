#include "connection.h"

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
TimerHandle_t pingTimer;

void ping() {
  if (!mqttClient.connected()) {
    return;
  }

  StaticJsonDocument<200> doc; // <- 200 bytes in the heap
  doc["type"] = "ping";
  char serializedJson[30];
  serializeJson(doc, serializedJson);

  mqttClient.publish(CFG_MQTT_UPSTREAM_TOPIC, 0, true, serializedJson);

  xTimerStart(pingTimer, 0);
}

void connectToMqtt() {
  ardprintf("Connecting to MQTT...");
  mqttClient.connect();
}

bool connectToWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  int wifiRetriesLeft = 50;

  char ssid[60] = CFG_WIFI_SSID;
  char password[60] = CFG_WIFI_PASSWORD;

  WiFi.begin(ssid, password);
  ardprintf("Station: Connecting to %s", ssid);

  while (WiFi.status() != WL_CONNECTED && wifiRetriesLeft > 0) {
    delay(100);
    wifiRetriesLeft -= 1;
  }

  if (wifiRetriesLeft <= 0 || WiFi.status() != WL_CONNECTED) {
    ardprintf("Station: Could not connect to WiFi.");
    return false;
  }
  
  ardprintf("Station: Connected to WiFi");
  return true;
}

void WiFiEvent(WiFiEvent_t event) {
    ardprintf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        ardprintf("WiFi connected");
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ardprintf("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
		    xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}


void onMqttConnect(bool sessionPresent) {
  ardprintf("Connected to MQTT.");
  ardprintf("Session present: %b", sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe(CFG_MQTT_DOWNSTREAM_TOPIC, 2);
  ardprintf("Subscribing at QoS 2, packetId: %d", packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  ardprintf("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  ardprintf("Subscribe acknowledged.");
  ardprintf("packetId: %d", packetId);
  ardprintf("qos: %d", qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  ardprintf("Unsubscribe acknowledged.");
  ardprintf("packetId: %d", packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char buffer[500];

  StaticJsonDocument<1000> doc; // <- a little more than 1000 bytes in the stack
  deserializeJson(doc, payload);
  
  StaticJsonDocument<1000> data; // <- a little more than 1000 bytes in the stack
  deserializeJson(data, doc["data"]);
  doc.clear();
  JsonObject obj = data.as<JsonObject>();

  const char * type = data["type"];
  ardprintf("type: %s", type);


  if (obj["payload"]) {
    serializeJson(obj["payload"], buffer);
  }

  if (strcmp(type, "config") == 0) {
    NVS.setString(NVS_CONFIG_KEY, String(buffer));

    // char * st = NVS.getCharArray(NVS_CONFIG_KEY);
    // if (st) {
    //   ardprintf("Config read back is: %s", st);
    // }
  } else if (strcmp(type, "config-request") == 0) {
    data.clear();
    doc.clear();
    data["type"] = "config";

    if (NVS.exists(NVS_CONFIG_KEY)) {
      char * st = NVS.getCharArray(NVS_CONFIG_KEY);
      data["payload"] = st;
    } else {
      ardprintf("Creating config from defaults");
      // create object into doc
      JsonArray d = doc.createNestedArray("presets");
      for (int i=0; i<sizeof(presets)/sizeof(presets[0]); i++) {
        d.add(presets[i]);
      }
      serializeJson(doc, buffer);
      ardprintf("%s buffer", buffer);
      // stringify it into data
      data["payload"] = buffer;
    }
    serializeJson(data, buffer);
    mqttClient.publish(CFG_MQTT_UPSTREAM_TOPIC, 0, true, buffer);
    ardprintf("Sent config %s", buffer);
  }
}

void onMqttPublish(uint16_t packetId) {
  ardprintf("Publish acknowledged.");
  ardprintf("packetId: %d", packetId);
}

void connectionSetup() {
  WiFi.disconnect();                                 // After restart router could still
  delay(500);                                        // keep old connection
  WiFi.mode(WIFI_STA);                               // This ESP is a station
  delay(500);                                        // ??
  WiFi.persistent(false);                            // Do not save SSID and password

  NVS.begin();

  // reconnect with timers to ensure we try every 2sek when connection is lost
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  // send ping to upstream mqtt once a minute
  pingTimer = xTimerCreate("pingTimer", pdMS_TO_TICKS(60000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(ping));
  xTimerStart(pingTimer, 0);

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(CFG_MQTT_HOST, MQTT_PORT);

  connectToWifi();
}