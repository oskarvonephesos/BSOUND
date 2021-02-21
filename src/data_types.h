//
//  data_types.h
//  BSOUND
//
//  Created by Richard Schwennicke on 1/6/21.
//
#ifndef data_types_h
#define data_types_h
#include "globaltypes.h"
typedef struct {
    MYFLT** value;
    int length;
    int index;
    int available;
} RNGBUF;
typedef struct {
    MYFLT * value;
    int length;
    int index;
    MYFLT read_index;
    MYFLT read_incr;
    MYFLT prv_y0;
    int linseg_rmns;
} DELAY_LINE;
RNGBUF* alloc_rngbuf(BSOUND * bsound, int length);
void dealloc_rngbuf(RNGBUF* buffer, BSOUND* bsound);
DELAY_LINE* alloc_delay_line(BSOUND* bsound, int length);
void dealloc_delay_line(DELAY_LINE* line, BSOUND* bsound);
#ifndef _STDLIB
#define _STDLIB
#include <stdlib.h>
#include <stdio.h>
#endif
#endif /* data_types_h */
