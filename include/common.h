
#ifndef __COMMON_H__
#define __COMMON_H__

// macro to string expansion for env variables
#define xstr(s) strs(s)
#define strs(s) #s

#define DEBUG_BUFFER_SIZE 150
#define sv DRAM_ATTR static volatile

#include <Arduino.h>

enum datamode_t { INIT = 0x1, HEADER = 0x2, DATA = 0x4,      // State for datastream
                  METADATA = 0x8,
                  STOPREQD = 0x80, STOPPED = 0x100, RESUMEREQD = 0x81
                } ;
extern datamode_t        datamode;                            // State of datastream

char* ardprintf( const char* format, ...);
void setdatamode(datamode_t newmode);

#define DEBUG 1                            // Debug on/off

#endif