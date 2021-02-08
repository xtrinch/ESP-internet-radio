
#ifndef __IR_H__
#define __IR_H__

#include "common.h"

extern uint16_t          ir_value;                         // IR code
extern uint32_t          ir_0;                           // Average duration of an IR short pulse
extern uint32_t          ir_1;                          // Average duration of an IR long pulse  

void scanIR();
void IRAM_ATTR isr_IR();

#endif