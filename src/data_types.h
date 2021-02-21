/**
      BSOUND
      a live audio processing tool
      designed to interact with acoustic instruments
    Copyright (C) <2021> <Richard Schwennicke>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    **/
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
