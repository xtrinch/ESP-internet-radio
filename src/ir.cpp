
#include "ir.h"

uint16_t          ir_value = 0;                        // IR code

// See if IR input is available.  Execute the programmed command.   
void scanIR()
{
  /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     */
    if (IrReceiver.decode()) {
        // Print a short summary of received data
        IrReceiver.printIRResultShort(&Serial);
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            // We have an unknown protocol, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
        Serial.println();

        IrReceiver.resume(); // Enable receiving of the next value
        /*
         * Check the received data
         */
        if (IrReceiver.decodedIRData.command == 0x11) {
            // do something
        }
    } else if (IrReceiver.results.overflow) {
        IrReceiver.results.overflow = false;
        // no need to call resume, this is already done by decode()
        Serial.println(F("Overflow detected"));
    }
}