//
//  programm state.h
//  BSOUND
//
//  Created by Richard Schwennicke on 1/3/21.
//
#include <stdlib.h>
#include <math.h>
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
///this sets the (default) latency
#ifndef BUF
#define BUF 256
#endif
///not currently in use
#ifndef HBUF
#define HBUF 512
#endif
///not currently in use
#ifndef CHANS
#define CHANS 2
#endif
#include "globaltypes.h"
struct _BSOUND* init_bsound(void);
struct _op_stack* init_head (void);
    
#endif /* programm_state_h */
