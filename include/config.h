
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.h"
#include <TridentTD_ESP32NVS.h>
#include <ArduinoJson.h>

#define NVS_CONFIG_KEY "configzy"

void configSetup();
void parseExistingConfig();
char * existingConfig();
bool configExists();
void setConfig(const char * buffer);
char * existingConfig();
char * hostFromConfig(int16_t preset);

extern char presets[12][100];

#endif