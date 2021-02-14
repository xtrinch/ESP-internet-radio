#ifndef __MAIN_H__
#define __MAIN_H__

#include "vs1053.h"
#include "common.h"
#include "tft-ili9341.h"
#include <WebServer.h>
#include "ir.h"
#include "stream-mp3.h"
#include "connection.h"
#ifdef MQTT_ENABLED
#include "connection-mqtt.h"
#endif

// Number of entries in the queue
#define QSIZ 400

#include <stdio.h>
#include <string.h>
#include <SPI.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <driver/adc.h>

extern int16_t           currentpreset;                   // Preset station playing
extern int16_t           newpreset;                       // Preset station playing

const char* changeState(const char* str);
void queuefunc(int func);
bool isPlaying();

#endif