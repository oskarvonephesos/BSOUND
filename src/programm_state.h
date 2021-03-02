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
#include <stdlib.h>
#include <math.h>
#include "globaltypes.h"
#ifndef programm_state_h
#define programm_state_h


#ifndef MY_2_PI
#define MY_2_PI (6.28318530718)
#endif
///this is the sample_rate
#ifndef SR
#define SR 44100.
#endif
///this sets the (default) latency on first startup;
///on further start-ups, buffer size is read from log.txt
#ifndef BUF
#define BUF 128
#endif
BSOUND* init_bsound(void);
OP_STACK* init_head (void);

#endif /* programm_state_h */
