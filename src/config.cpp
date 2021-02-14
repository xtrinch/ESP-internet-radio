#include "config.h"

// default presets
// the only way to configure them is via MQTT or changing it here
char presets[12][100] = {
  "109.206.96.34:8100",
  "airspectrum.cdnstream1.com:8114/1648_128",
  "us2.internet-radio.com:8050",
  "airspectrum.cdnstream1.com:8000/1261_192",
  "airspectrum.cdnstream1.com:8008/1604_128",
  "us1.internet-radio.com:8105",
  "icecast.omroep.nl:80/radio1-bb-mp3",
  "205.164.62.15:10032",
  "skonto.ls.lv:8002/mp3",
  "94.23.66.155:8106",
  "ihr/IHR_IEDM",
  "ihr/IHR_TRAN"
};

char * hostFromConfig(int16_t preset) {
  return presets[preset];
}

void configSetup() {
  NVS.begin();
  parseExistingConfig();
}

void parseExistingConfig() {
  char * conf = existingConfig();
  if (!conf) {
    return;
  }

  ardprintf("Found existing config. Copying into memory...");

  StaticJsonDocument<1000> doc; // <- a little more than 1000 bytes in the stack
  deserializeJson(doc, conf);
  if (doc["presets"]) {
    JsonArray array = doc["presets"].as<JsonArray>();
    int i=0;
    for(JsonVariant v : array) {
      strncpy(presets[i], v.as<char *>(), 100);
      i++;
    }
  }
}

bool configExists() {
  return NVS.exists(NVS_CONFIG_KEY);
}

void setConfig(const char * buffer) {
  NVS.setCharArray(NVS_CONFIG_KEY, buffer);

  parseExistingConfig();
}

char * existingConfig() {
  if (NVS.exists(NVS_CONFIG_KEY)) {
    return NVS.getCharArray(NVS_CONFIG_KEY);
  }
  return NULL;
}