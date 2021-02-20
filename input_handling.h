//
//  input_handling.h
//  BSOUND
//
//  Created by Richard Schwennicke on 1/4/21.
//
#include "globaltypes.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef input_handling_h
#define input_handling_h
typedef enum _USR_IN{
    DELAY, // delay opcodes
    PPDEL,
    TAPE,
    MULTITAP,
    CLOUD, // partikkel opcodes
    SHIMMER,
    TRANSPOSE,
    REVERB, // resonator opcodes
    PEDAL,
    RINGMOD, //moddemod opcodes
    MODDEMOD,
    CRUSH,
    BBD,
    DELETE,
    MANUAL,
    CLEAR,
    PREFERENCES_MENU,
    NUM_OPTIONS,
    //manipulate stack
    EMPTY = -1,
    ERROR = -2
}USR_IN;
void* input_handler(void* id );
void error_message(char* message, struct _BSOUND* bsound);

#endif /* input_handling_h */
