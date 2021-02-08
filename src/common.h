
#ifndef __COMMON_H__
#define __COMMON_H__

#define ir_pin -1
#define tft_cs_pin 15 // Display SPI version
#define tft_dc_pin 2 // Display SPI version
#define tft_scl_pin 18 // Display I2C version
#define tft_sda_pin 23 // Display I2C version
#define tft_bl_pin -1 // Display backlight
#define tft_blx_pin -1 // Display backlight (inversed logic)
#define tft_touch_irq_pin 26
#define vs_cs_pin 22 // VS1053 pins
#define vs_dcs_pin 32
#define vs_dreq_pin 4
#define vs_shutdown_pin -1 // Amplifier shut-down pin
#define vs_shutdownx_pin -1 // Amplifier shut-down pin (inversed logic)
#define spi_sck_pin      18
#define spi_miso_pin     19
#define spi_mosi_pin     23

#define DEBUG_BUFFER_SIZE 150
// Max number of presets in preferences
#define MAXPRESETS 200
// Access point name if connection to WiFi network fails.  Also the hostname for WiFi and OTA.
// Note that the password of an AP must be at least as long as 8 characters.
// Also used for other naming.
#define NAME "ESP32Radio"
// Define the version number, also used for webserver as Last-Modified header and to
// check version for update.  The format must be exactly as specified by the HTTP standard!
#define VERSION     "Thu, 21 Jan 2021 15:20:00 GMT"
// Max. number of NVS keys in table
#define MAXKEYS 200
#define sv DRAM_ATTR static volatile

#include <Arduino.h>

enum datamode_t { INIT = 0x1, HEADER = 0x2, DATA = 0x4,      // State for datastream
                  METADATA = 0x8,
                  STOPREQD = 0x80, STOPPED = 0x100
                } ;
extern datamode_t        datamode;                            // State of datastream

char*       dbgprint( const char* format, ...);
void        chomp(String &str);
void setdatamode(datamode_t newmode);

#define DEBUG 1                            // Debug on/off

#endif