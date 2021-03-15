#ifndef GUI_UTILITIES_H
#define GUI_UTILITIES_H
#include "input_handling.h"
#include "globaltypes.h"
#include "opcodes.h"
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/*
This includes forward declarations of GUI private functions.
GUI public functions are forward declared in input_handling.h
*/

//these interact with user input
int32_t autocomplete(char* instring, int32_t length, int16_t option);
//parse is public for test_sound
//encapsulation utilities
char* getname(OP_STACK* item);
int32_t getlength(OP_STACK* item);
int16_t getnumdisplaytypes(OP_STACK* item);
int16_t getmaxval(OP_STACK* item, int16_t num_attr);
int16_t getminval(OP_STACK* item, int16_t num_attr);
char* getdisplayname(OP_STACK* item, int16_t num_attr);
char getdisplaytype(OP_STACK* item, int16_t num_attr);
//display routines page 1
int32_t display_stack(BSOUND* bsound, int32_t max_x, int32_t max_y);
void display_attr(int16_t *attr, int32_t index, USR_IN type, int32_t *display_loc, char display_type);
//this is the stack manipulation
int32_t insert_op(BSOUND* bsound, COMMAND* command);
void delete_item(BSOUND* bsound, OP_STACK* cursor);
//these are routines to load and save
OP_STACK* load_st(BSOUND* bsound, int16_t* print_loc);
void save_st(BSOUND* bsound, int16_t* print_loc);
#endif
