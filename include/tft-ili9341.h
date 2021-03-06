#ifndef __TFT_ILI9341_H__
#define __TFT_ILI9341_H__

#include "common.h"
#include "main.h"

#include <TFT_eSPI.h>
#include <SPIFFS.h>

// Color definitions for the TFT screen
// TFT has bits 6 bits (0..5) for RED, 6 bits (6..11) for GREEN and 4 bits (12..15) for BLUE.
#define BLACK   ILI9341_BLACK
#define BLUE    ILI9341_BLUE
#define RED     ILI9341_RED
#define GREEN   ILI9341_GREEN
#define CYAN    GREEN | BLUE
#define MAGENTA RED | BLUE
#define YELLOW  RED | GREEN
#define WHITE   RED | BLUE | GREEN

// #define AA_FONT_SMALL "fonts/NotoSansBold15" // 15 point sans serif bold
// #define AA_FONT_LARGE "fonts/NotoSansBold36" // 36 point sans serif bold
// #define AA_FONT_MEDIUM "fonts/NotoSansMonoSCB20" // 36 point sans serif bold
#define SMOOTH_FONT_1 "fonts/Latin-Hiragana-24" // 36 point sans serif bold
// #define SMOOTH_FONT_2 "fonts/Final-Frontier-28" // 36 point sans serif bold

// Time-out [sec] for blanking TFT display (BL pin)
#define BL_TIME 45

extern uint16_t          bltimer;                         // Backlight time-out counter
extern TFT_eSPI     tft;                                  // For instance of display driver

void request_update();
bool display_begin();
void IRAM_ATTR blset(bool enable);
void tftset(uint16_t inx, const char *str);
bool refreshDisplay();

#endif