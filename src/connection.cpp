#include "connection.h"

TimerHandle_t wifiReconnectTimer;

bool connectToWifi() {
  if (WiFi.isConnected()) {
    return true;
  }

  char ssid[60] = CFG_WIFI_SSID;
  char password[60] = CFG_WIFI_PASSWORD;

  WiFi.begin(ssid, password);
  return true;
}

void WiFiEvent(WiFiEvent_t event) {
  // ardprintf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      ardprintf("WiFi connected");

      #ifdef MQTT_ENABLED
      connectToMqtt();
      #endif

      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      ardprintf("WiFi lost connection");

      #ifdef MQTT_ENABLED
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      #endif
      
      xTimerStart(wifiReconnectTimer, 0);
      break;
    default:
      break;
  }
}

void connectionSetup() {
  WiFi.disconnect();                                 // After restart router could still
  delay(500);                                        // keep old connection
  WiFi.mode(WIFI_STA);                               // This ESP is a station
  delay(500);                                        // ??
  WiFi.persistent(false);                            // Do not save SSID and password

  // reconnect with timers to ensure we try every 2sek when connection is lost
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);
  connectToWifi();

  while(!WiFi.isConnected()) {
    delay(100);
  }
}