
#ifndef __STREAM_MP3_H__
#define __STREAM_MP3_H__

#include "common.h"
#include <base64.h>
#include "main.h"
#include "config.h"

// Size of metaline buffer
#define METASIZ 1024

enum qdata_type { QDATA, QSTARTSONG, QSTOPSONG };   // datatyp in qdata_struct
typedef struct qdata_struct
{
  int datatyp;                                      // Identifier
  __attribute__((aligned(4))) uint8_t buf[32];      // Buffer for chunk
} qdata_struct;

extern WiFiClient        mp3client;                           // An instance of the mp3 client, also used for OTA
extern uint32_t          totalcount;                          // Counter mp3 data
extern qdata_struct      outchunk;                            // Data to queue
extern qdata_struct      inchunk;                             // Data from queue
extern QueueHandle_t     dataqueue;                           // Queue for mp3 datastream

void        mp3loop();

#endif