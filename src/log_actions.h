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
