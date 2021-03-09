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
#include "globaltypes.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef input_handling_h
#define input_handling_h

void* input_handler(void* id );
int16_t which_attr_is_skip(int16_t attr_num);
void error_message(const char* message, BSOUND* bsound);

COMMAND* parse(char* line, int32_t length);
int32_t insert_op(BSOUND* bsound, COMMAND* command);
#endif /* input_handling_h */
