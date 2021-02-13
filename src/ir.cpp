
#include "ir.h"

IRrecv IrReceiver(IR_PIN);

void setupIR() {
  if (IR_PIN >= 0) {
    ardprintf("Enable pin %d for IR", IR_PIN);
    IrReceiver.enableIRIn();  // Start the receiver
  }
}

// See if IR input is available.  Execute the programmed command.   
void scanIR()
{
    // Check if received data is available and if yes, try to decode it.
    if (IrReceiver.decode()) {
      // wake up the screen if IR command is received
      blset(true);

      // DEBUG data
      // // Print a short summary of received data
      // IrReceiver.printIRResultShort(&Serial);
      // if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      //   // We have an unknown protocol, print more info
      //   IrReceiver.printIRResultRawFormatted(&Serial, true);
      // }
      
      IrReceiver.resume(); // Enable receiving of the next value
      
      // if command received is a repeat command
      if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
        return;
      }

      switch(IrReceiver.decodedIRData.command) {
        case 0x43:
          ardprintf("Received command forward");
          changeState("preset=next");
          break;
        case 0x40:
          ardprintf("Received command backward");
          changeState("preset=prev");
          break;
        case 0x44:
          ardprintf("Received command play/stop");
          if (isPlaying()) {
            changeState("stop");
          } else {
            changeState("resume");
          }
          break;
      }
    } else if (IrReceiver.results.overflow) {
      IrReceiver.results.overflow = false;
      // no need to call resume, this is already done by decode()
    }
}