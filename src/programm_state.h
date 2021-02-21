//
//  programm state.h
//  BSOUND
//
//  Created by Richard Schwennicke on 1/3/21.
//
#include <stdlib.h>
#include <math.h>
#include "globaltypes.h"
#ifndef programm_state_h
#define programm_state_h


#ifndef MY_2_PI
#define MY_2_PI (6.28318530718)
#endif
///this is the precision used within opcodes
#ifndef MYFLT
typedef double MYFLT;
#endif
#ifndef SR
#define SR 44100.
#endif
///this sets the (default) latency on first startup
#ifndef BUF
#define BUF 256
#endif
BSOUND* init_bsound(void);
op_stack* init_head (void);

#endif /* programm_state_h */
