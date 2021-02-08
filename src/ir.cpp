
#include "ir.h"

uint16_t          ir_value = 0;                        // IR code
uint32_t          ir_0 = 550;                          // Average duration of an IR short pulse
uint32_t          ir_1 = 1650;                         // Average duration of an IR long pulse


//                                     S C A N I R                  

// See if IR input is available.  Execute the programmed command.   

void scanIR()
{
  // TODO: use hardcoded keys

  // char        mykey[20];                                  // For numerated key
  // String      val;                                        // Contents of preference entry
  // const char* reply;                                      // Result of analyzeCmd
  // if (ir_value)                                          // Any input?
  // {
  //   sprintf(mykey, "ir_%04X", ir_value);                // Form key in preferences
  //   if (nvssearch(mykey))
  //   {
  //     val = nvsgetstr(mykey);                           // Get the contents
  //     dbgprint("IR code %04X received. Will execute %s",
  //                ir_value, val.c_str());
  //     reply = analyzeCmd(val.c_str());                  // Analyze command and handle it
  //     dbgprint(reply);                                  // Result for debugging
  //   }
  //   else
  //   {
  //     dbgprint("IR code %04X received, but not found in preferences!  Timing %d/%d",
  //                ir_value, ir_0, ir_1);
  //   }
  //   ir_value = 0;                                         // Reset IR code received
  // }
}



//                                          I S R _ I R             

// Interrupts received from VS1838B on every change of the signal.  
// Intervals are 640 or 1640 microseconds for data.  syncpulses are 3400 micros or longer.      
// Input is complete after 65 level changes.                        
// Only the last 32 level changes are significant and will be handed over to common data.       

void IRAM_ATTR isr_IR()
{
  sv uint32_t      t0 = 0;                         // To get the interval
  sv uint32_t      ir_locvalue = 0;                // IR code
  sv int           ir_loccount = 0;                // Length of code
  uint32_t         t1, intval;                     // Current time and interval since last change
  uint32_t         mask_in = 2;                    // Mask input for conversion
  uint16_t         mask_out = 1;                   // Mask output for conversion

  t1 = micros();                                    // Get current time
  intval = t1 - t0;                                // Compute interval
  t0 = t1;                                         // Save for next compare
  if (( intval > 300)&&(intval < 800))        // Short pulse?
  {
    ir_locvalue = ir_locvalue << 1;                // Shift in a "zero" bit
    ir_loccount++;                                 // Count number of received bits
    ir_0 =(ir_0 * 3 + intval)/ 4;              // Compute average durartion of a short pulse
  }
  else if (( intval > 1400)&&(intval < 1900)) // Long pulse?
  {
    ir_locvalue =(ir_locvalue << 1)+ 1;        // Shift in a "one" bit
    ir_loccount++;                                 // Count number of received bits
    ir_1 =(ir_1 * 3 + intval)/ 4;              // Compute average durartion of a short pulse
  }
  else if (ir_loccount == 65)                     // Value is correct after 65 level changes
  {
    while(mask_in)                               // Convert 32 bits to 16 bits
    {
      if (ir_locvalue & mask_in)                  // Bit set in pattern?
      {
        ir_value |= mask_out;                      // Set set bit in result
      }
      mask_in <<= 2;                               // Shift input mask 2 positions
      mask_out <<= 1;                              // Shift output mask 1 position
    }
    ir_loccount = 0;                               // Ready for next input
  }
  else
  {
    ir_locvalue = 0;                               // Reset decoding
    ir_loccount = 0 ;
  }
}
