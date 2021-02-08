#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>
#include "defaultprefs.h"
#include <VS1053.h>
#include "common.h"
#include "tft-ili9341.h"
#include "webserver.h"
#include "ir.h"
#include "stream-mp3.h"

// macro to string expansion for env variables
#define xstr(s) strs(s)
#define strs(s) #s

#define CFG_WIFI_SSID xstr(WIFI_SSID)
#define CFG_WIFI_PASSWORD xstr(WIFI_PASSWORD)
#define ILI9341                      // ILI9341 240*320
// Number of entries in the queue
#define QSIZ 400

#include <Arduino.h>
#include <nvs.h>
#include <ESPmDNS.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <SPI.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <esp_partition.h>
#include <driver/adc.h>
#include "defaultprefs.h"

typedef struct keyname_t                                      // For keys in NVS
{
  char      Key[16];                                // Max length is 15 plus delimeter
} keyname_t;

extern bool              NetworkFound;                // True if WiFi network connected
extern char              nvskeys[MAXKEYS][16];          // Space for NVS keys
extern hw_timer_t*       timer;                        // For timer
extern int16_t           currentpreset;                   // Preset station playing
extern int16_t           newpreset;                       // Preset station playing
extern int8_t            playing;                     // 1 if radio is playing (for MQTT)

const char* analyzeCmd(const char* str);
const char* analyzeCmd(const char* par, const char* val);
void queuefunc(int func);

#endif