
#ifndef __IR_H__
#define __IR_H__

#include "common.h"
#include <IRremote.h>
#include "main.h"

extern uint16_t          ir_value;                         // IR code

void scanIR();

#endif