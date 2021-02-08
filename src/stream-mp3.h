
#ifndef __STREAM_MP3_H__
#define __STREAM_MP3_H__

#include "common.h"
#include <base64.h>
#include "main.h"

// Size of metaline buffer
#define METASIZ 1024

enum qdata_type { QDATA, QSTARTSONG, QSTOPSONG };   // datatyp in qdata_struct
typedef struct qdata_struct
{
  int datatyp;                                      // Identifier
  __attribute__((aligned(4))) uint8_t buf[32];      // Buffer for chunk
} qdata_struct;

extern int               mbitrate;                            // Measured bitrate
extern char              metalinebf[METASIZ + 1];             // Buffer for metaline/ID3 tags
extern int16_t           metalinebfx;     
extern String            host;                                // The URL to connect to or file to play
extern WiFiClient        mp3client;                           // An instance of the mp3 client, also used for OTA
extern String            playlist;                            // The URL of the specified playlist
extern uint32_t          totalcount;                      // Counter mp3 data
extern int               datacount;                           // Counter databytes before metadata
extern qdata_struct      outchunk;                            // Data to queue
extern qdata_struct      inchunk;                             // Data from queue
extern uint8_t*          outqp;                // Pointer to buffer in outchunk
extern uint32_t          max_mp3loop_time;                // To check max handling time in mp3loop (msec)
extern QueueHandle_t     dataqueue;                           // Queue for mp3 datastream
extern bool              hostreq;                     // Request for new host
extern String            icystreamtitle;                      // Streamtitle from metadata
extern String            icyname;                             // Icecast station name
extern char presets[12][100];

void        stop_mp3client ();
void        showstreamtitle(const char* ml, bool full = false);
void handlebyte_ch(uint8_t b);
bool connecttohost();
void        mp3loop();

#endif