#include "common.h"

datamode_t        datamode;                            // State of datastream

// Send a line of info to serial output.  Works like vsprintf(), but checks the DEBUG flag.     
// Print only if DEBUG flag is true.  Always returns the formatted string.           
char* ardprintf(const char* format, ... )
{
  static char sbuf[DEBUG_BUFFER_SIZE];               // For debug lines
  va_list varArgs;                                   // For variable number of params

  va_start(varArgs, format);                       // Prepare parameters
  vsnprintf(sbuf, sizeof(sbuf), format, varArgs);  // Format the message
  va_end(varArgs);                                 // End of using parameters
  if (DEBUG)                                        // DEBUG on?
  {
    Serial.print("D: ");                           // Yes, print prefix
    Serial.println(sbuf);                          // and the info
  }
  return sbuf;                                       // Return stored string
}

// Change the datamode and show in debug for testing.               
void setdatamode(datamode_t newmode ) {
  // ardprintf("Change datamode from 0x%03X to 0x%03X",
  //           (int)datamode, (int)newmode);
  datamode = newmode ;
}
