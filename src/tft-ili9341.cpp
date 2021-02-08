#include "tft-ili9341.h"
#include <XPT2046_Touchscreen.h>

// hardware ESP pins must be used
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

XPT2046_Touchscreen ts(TOUCH_CS, tft_touch_irq_pin);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

uint16_t width = 320;
uint16_t height = 240;

String icyName = "";
String streamTitle = "";

uint16_t          bltimer = 0;                         // Backlight time-out counter
bool update_req = false;

void touch_ISR();
void touch_calibrate();

//* Function that are called from spftask.                          

void request_update () {
  update_req = true;                   // and request flag
}

// Request to display a segment on TFT.  Version for char* and String parameter.     
void tftset(uint16_t inx, const char *str )
{
  switch(inx) {
    case 1:
      streamTitle = String (str);
      break;
    case 2:
      icyName = String (str);
      break;
  }
  update_req = true;                   // and request flag
}

unsigned long last_interrupt_time = 0;

// Check if tft refresh is requested.                               
bool refreshDisplay()
{
  // tirqTouched() is much faster than touched().  For projects where other SPI chips
  // or other time sensitive tasks are added to loop(), using tirqTouched() can greatly
  // reduce the delay added to loop() when the screen has not been touched.
  if (ts.tirqTouched()) {
    if (ts.touched()) {
      if (millis() - last_interrupt_time > 500) {
        TS_Point p = ts.getPoint();
        uint16_t x = p.x;
        uint16_t y = p.y;
        tft.convertRawXY(&x, &y);
        // tft.drawCircle(x, y, 1, TFT_WHITE);
        last_interrupt_time = millis();
        // dbgprint("tch: %d %d", x, y);
        // if x on icon 120,60 with wxh 32x32, we use a safety safety margin of 10 on sides
        if (x > 120 && x < 190 && y < 200 && y > 160) {
          // request stop or play
          if (!playing) {
            analyzeCmd("resume");
          } else {
            analyzeCmd("stop");
          }
        } else if (x > 50 && x < 110 && y < 200 && y > 160) {
          analyzeCmd("preset=prev");
        } else if (x > 230 && x < 290 && y < 200 && y > 160) {
          analyzeCmd("preset=next");
        }
      }
    }
  }
  
  if (!update_req) {
    return false;  
  }

  sprite.createSprite(width-40, height-40);
  sprite.fillSprite(TFT_BLACK);

  sprite.setTextWrap(false); // Wrap on width

  // icy name
  sprite.setTextColor (TFT_YELLOW);     
  char displayIcyName[60];
  snprintf(displayIcyName, 60, "%d: %s", currentpreset, icyName.c_str());
  sprite.drawString(displayIcyName, 0, 0);       

  sprite.setTextWrap(true); // Wrap on width

  // stream title
  sprite.setTextColor (TFT_WHITE);
  sprite.drawRect(0, 50, width - 40, 50, TFT_BLACK); // handle overflow of icy name                
  sprite.drawString(streamTitle, 0, 40);  

  sprite.fillRect(61, 160, 5, 32, TFT_YELLOW);
  sprite.fillTriangle(56, 160, 26, 175, 56, 190, TFT_YELLOW);
  if (datamode == STOPPED) {
    sprite.fillTriangle(120, 160, 150, 175, 120, 190, TFT_GREEN);
  } else {
    sprite.fillRect(120, 160, 32, 32, TFT_RED); 
  }
  sprite.fillRect(207, 160, 5, 32, TFT_YELLOW);
  sprite.fillTriangle(216, 160, 246, 175, 216, 190, TFT_YELLOW);

  update_req = false;  // Reset request
  sprite.pushSprite(20, 20);
  return true;                                         // Not a single request
}

bool display_begin()
{
  SPIFFS.begin(); // Init flash filesystem
  ts.begin(SPI);
  ts.setRotation(3);
  tft.begin();                                                    // Init TFT interface

  tft.setRotation(3);                            // Use landscape format
  tft.fillScreen(BLACK);                        // Clear screen
  tft.setTextColor(WHITE);                      // Info in white

  sprite.setColorDepth(8);

  // Calibrate the touch screen and retrieve the scaling factors
  // touch_calibrate();

  uint16_t calData[5] = { 316, 3425, 212, 3410, 6 };
  tft.setTouch(calData);
  sprite.loadFont(SMOOTH_FONT_1);

  return true ;
}

// Enable or disable the TFT backlight if configured.               
// May be called from interrupt level.                              
void IRAM_ATTR blset(bool enable )
{
  if (tft_bl_pin >= 0)                      // Backlight for TFT control?
  {
    digitalWrite(tft_bl_pin, enable);      // Enable/disable backlight
  }
  if (tft_blx_pin >= 0)                     // Backlight for TFT (inversed logic) control?
  {
    digitalWrite(tft_blx_pin, !enable);    // Enable/disable backlight
  }
  if (enable )
  {
    bltimer = 0;                                       // Reset counter backlight time-out
  }
}

// Code to run a screen calibration, not needed when calibration values set in setup()
void touch_calibrate()
{
  uint16_t calData[5];

  // Calibrate
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Touch corners as indicated");

  tft.setTextFont(1);
  tft.println();

  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  Serial.println(); Serial.println();
  Serial.println("// Use this calibration code in setup():");
  Serial.print("  uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println(); Serial.println();

  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("Calibration complete!");
  tft.println("Calibration code sent to Serial port.");

  delay(4000);
}