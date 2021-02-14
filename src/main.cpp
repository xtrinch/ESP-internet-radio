#include "main.h"

// Forward declaration and prototypes of various functions.         
void playtask(void * parameter);             // Task to play the stream
void spftask(void * parameter);              // Task for special functions
void claimSPI(const char* p);                // Claim SPI bus for exclusive access
void releaseSPI();                           // Release the claim
void timer100();
void changeState(const char* par, const char* val);

// The object for the MP3 player
VS1053* vs1053player;

// Global variables
TaskHandle_t      maintask;                            // Taskhandle for main task
TaskHandle_t      xplaytask;                           // Task handle for playtask
TaskHandle_t      xspftask;                            // Task handle for special functions
SemaphoreHandle_t SPIsem = NULL;                       // For exclusive SPI usage
hw_timer_t*       hw_timer = NULL;                     // For timer
QueueHandle_t     spfqueue;                            // Queue for special functions
int16_t           currentpreset = -1;                  // Preset station playing
int16_t           newpreset = 0;                       // Preset station playing
int               mbitrate;                            // Measured bitrate

void setup() {
  Serial.begin(115200);                        
  while(!Serial);

  // Print some memory and sketch info
  ardprintf("Starting ESP32-radio running on CPU %d at %d MHz. Free memory %d",
             xPortGetCoreID(),
             ESP.getCpuFreqMHz(),
             ESP.getFreeHeap());                       // Normally about 170 kB
  maintask = xTaskGetCurrentTaskHandle();               // My taskhandle
  SPIsem = xSemaphoreCreateMutex();                    // Semaphore for SPI bus
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);

  display_begin();

  vs1053player = new VS1053(VS_CS_PIN, VS_DCS_PIN, VS_DREQ_PIN, -1, -1);
  
  setupIR();

  if (TFT_BL >= 0) {                      // Backlight for TFT control?
    pinMode(TFT_BL, OUTPUT);           // Yes, enable output
  }
  blset(true);                                       // Enable backlight (if configured)
  
  // has to be called before wifi setup
  #ifdef MQTT_ENABLED
  mqttSetup();
  #endif

  // setup wifi
  connectionSetup();

  // setup nvs config
  configSetup();

  // Initialize VS1053 player
  vs1053player->begin();                                

  if (!WiFi.isConnected()) {                                
    currentpreset = -1;                                 // No network: do not start radio
  }
  hw_timer = timerBegin(0, 80, true);                   // User 1st timer with prescaler 80
  timerAttachInterrupt(hw_timer, &timer100, true);      // Call hw_timer100() on hw_timer alarm
  timerAlarmWrite(hw_timer, 100000, true);              // Alarm every 100 msec
  timerAlarmEnable(hw_timer);                           // Enable the timer

  outchunk.datatyp = QDATA;                             // This chunk dedicated to QDATA
  dataqueue = xQueueCreate(QSIZ, sizeof(qdata_struct)); // Create queue for communication

  xTaskCreatePinnedToCore (
    playtask,                                             // Task to play data in dataqueue.
    "Playtask",                                           // name of task.
    1600,                                                 // Stack size of task
    NULL,                                                 // parameter of the task
    2,                                                    // priority of the task
    &xplaytask,                                           // Task handle to keep track of created task
    0);                                                 // Run on CPU 0
  xTaskCreate (
    spftask,                                              // Task to handle special functions.
    "Spftask",                                            // name of task.
    2048,                                                 // Stack size of task
    NULL,                                                 // parameter of the task
    1,                                                    // priority of the task
    &xspftask);                                         // Task handle to keep track of created task
}

// Main loop of the program.                                        
void loop() {
  mp3loop();                                       // Do mp3 related actions
  scanIR();                                        // See if IR input
  mp3loop();                                       // Do more mp3 related actions
}

// Claim the SPI bus.  Uses FreeRTOS semaphores.                    
// If the semaphore cannot be claimed within the time-out period, the function continues without
// claiming the semaphore.  This is incorrect but allows debugging. 
void claimSPI(const char* p) {
  uint32_t           count = 0;                           // Wait time in ticks
  static const char* old_id = "none";                     // ID that holds the bus

  while(xSemaphoreTake(SPIsem, (TickType_t) 10) != pdTRUE) {    // Claim SPI bus, block for 10 ticks
    if (count++ > 50 ) {
      ardprintf("SPI semaphore not taken within %d ticks by CPU %d, id %s",
                 count * 10,
                 xPortGetCoreID(),
                 p);
      ardprintf("Semaphore is claimed by %s", old_id);
    } if (count >= 200 ) {
      // restart, there is no way out of this
      ESP.restart();
    }
  }
  old_id = p;                                              // Remember ID holding the semaphore
}

// Free the the SPI bus.  Uses FreeRTOS semaphores.                 
void releaseSPI() {
  if (xSemaphoreGive(SPIsem) != pdTRUE) {
    ardprintf("Error releasing SPI.");
  }
}

// Queue a special function for the play task.                      
void queuefunc(int func) {
  qdata_struct     specchunk;                         // Special function to queue

  specchunk.datatyp = func;                           // Put function in datatyp
  xQueueSendToFront(dataqueue, &specchunk, 200);    // Send to queue (First Out)
}

// Called every 10 seconds.                        
void IRAM_ATTR timer10sec() {
  static uint32_t oldtotalcount = 7321;          // Needed for change detection
  uint32_t        bytesplayed;                   // Bytes send to MP3 converter

  if (isPlaying()) {
    bytesplayed = totalcount - oldtotalcount;   // Nunber of bytes played in the 10 seconds
    oldtotalcount = totalcount;                 // Save for comparison in next cycle
    if (bytesplayed != 0) {                     // Still playing?
      // Bitrate in kbits/s is bytesplayed / 10 / 1000 * 8
      mbitrate = (bytesplayed + 625) / 1250;   // Measured bitrate
    }
  }
}

// Called every 100 msec on interrupt level, so must be in IRAM and no lengthy operations       
// allowed.                                                         
void IRAM_ATTR timer100() {
  sv int16_t   count10sec = 0;                  // Counter for activatie 10 seconds process

  if (++count10sec == 100) {                    // 10 seconds passed?
    timer10sec();                                // Yes, do 10 second procedure
    count10sec = 0;                             // Reset count
  }
  if ((count10sec % 10) == 0) {                // One second over?
    if (++bltimer == BL_TIME)                  // Time to blank the TFT screen?
    {
      bltimer = 0;                              // Yes, reset counter
      if (!isPlaying()) {
        blset(false);                           // Disable TFT (backlight)
      }
    }
  }
}

// Handling of the various commands from remote webclient, Serial or MQTT.           
// Version for handling string with: <parameter>=<value>            
void changeState(const char* str) {
  ardprintf("Command: %s", str);

  char strCpy [30];                              // make a copy, we shouldn't write into arguments
  strncpy(strCpy, str, 30);

  char* value = strstr(strCpy, "=");                  // See if command contains a "="
  if (value) {
    *value = '\0';                             // Separate command from value
    changeState(str, value + 1);        // Analyze command and handle it
    *value = '=';                              // Restore equal sign
  }
  else {
    changeState(str, "0");              // No value, assume zero
  }
}
         
// changes state of the player; examples:
//   preset     = 12                        // Select start preset to connect to 
//   preset     = next                      // Select start preset to connect to     
//   preset     = prev                      // Select start preset to connect to     
//   volume     = 95                        // Percentage between 0 and 100          
//   stop                                   // Stop playing         
//   resume                                 // Resume playing       
void changeState(const char* argument, const char* value) {
  blset(true);                                    // Enable backlight of TFT

  if (strstr(argument, "preset") != NULL) {          // change preset
    if (strcmp(value, "prev") == 0) {               // preset=prev
      if (currentpreset - 1 >= 0) {
        newpreset = currentpreset - 1;
      }
    } else if (strcmp(value, "next") == 0) {        // preset=next
      if (currentpreset + 1 < sizeof(presets)/sizeof(presets[0])) {
        newpreset = currentpreset + 1;
      }
    } else {
      newpreset = atoi(value);                          // Otherwise set station
    }
    setdatamode(STOPREQD);                          // Force stop MP3 player
  } else if (strcmp(argument, "stop") == 0) {                  // (un)Stop requested?
    setdatamode(STOPREQD);                          // Request STOP
  } else if (strcmp(argument, "resume") == 0) {
    setdatamode(RESUMEREQD);                          // Request RESUME
  } else {
    ardprintf("called with illegal parameter: %s", argument);
  }
  return; 
}

// Play stream data from input queue.                               
// Handle all I/O to VS1053B during normal playing.                 
void playtask(void * parameter) {
  while(true) {
    if (xQueueReceive(dataqueue, &inchunk, 5)) {
      while(!vs1053player->data_request()) {                        // If FIFO is full..
        vTaskDelay(1);                                              // Yes, take a break
      }
      switch(inchunk.datatyp) {                                     // What kind of chunk?
        case QDATA:
          claimSPI("chunk");                                         
          vs1053player->playChunk(inchunk.buf, sizeof(inchunk.buf)); // DATA, send to player
          releaseSPI();                                              
          totalcount += sizeof(inchunk.buf);                         // Count the bytes
          break;
        case QSTARTSONG:
          claimSPI("startsong");                                     
          vs1053player->startSong();                                 // START, start player
          releaseSPI();                                              
          break;
        case QSTOPSONG:
          request_update();
          claimSPI("stopsong");                           
          vs1053player->setVolume(0);                               // Mute
          vs1053player->stopSong();                                 // STOP, stop player
          releaseSPI();                                            
          while(xQueueReceive(dataqueue, &inchunk, 0));             // Flush rest of queue
          vTaskDelay(500 / portTICK_PERIOD_MS);                     // Pause for a short time
          break;
        default:
          break;
      }
    }
    //esp_task_wdt_reset();                                        // Protect against idle cpu
  }
}

// Handles display of text, time and volume on TFT.                 
// This task runs on a low priority.                                
void spftask(void * parameter) {
  while(true) {
    claimSPI("hspectft");                                 
    refreshDisplay();                                          // Yes, TFT refresh necessary
    releaseSPI();                                            
    claimSPI("hspec");                                     
    vs1053player->setVolume(90);                               // Unmute
    releaseSPI();                                              
    
    // highly necessary, as wi-fi will intermittently stop working without it!
    vTaskDelay(100 / portTICK_PERIOD_MS);                       // Pause for a short time
  }
}

bool isPlaying() {
  return datamode & (INIT | HEADER | DATA | METADATA);
}