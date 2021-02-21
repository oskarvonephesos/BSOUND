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
#ifndef log_actions_h
#define log_actions_h
#include "globaltypes.h"
#include <stdio.h>
typedef enum{
    FIRST_LOAD,
    NORMAL_LOAD,
    LOG_ERROR = -1
} LOG_STATE;
LOG_STATE init_log(const char* programm_location, BSOUND* bsound);
void save_to_log(const char* programm_loc, BSOUND* bsound);
#endif /* log_actions_h */
