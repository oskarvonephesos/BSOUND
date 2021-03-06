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
#include "input_handling.h"
#include "opcodes.h"
#include <string.h>
#include <pthread.h>
#include <curses.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#define IN_SIZE 7
#define NUM_ARGUMENTS 3
char save_slots[4][32]={"esja", "grettisgata", "harpa", "reykjadalur"};
#ifndef NUM_SAVE_SLOTS
#define NUM_SAVE_SLOTS 4
#endif
COMMAND* parse(char* line, int32_t length);
int32_t autocomplete(char* instring, int32_t length, int16_t option);
int32_t insert_op(BSOUND* bsound, COMMAND* command);
char* getname(OP_STACK* item);
int32_t getlength(OP_STACK* item);
int32_t display_stack(BSOUND* bsound, int32_t max_x, int32_t max_y);
void delete_item(BSOUND* bsound, OP_STACK* cursor);
OP_STACK* load_st(BSOUND* bsound, int16_t* print_loc);
void save_st(BSOUND* bsound, int16_t* print_loc);
void display_preferences_menu(BSOUND* bsound, int16_t* print_loc);

int16_t which_attr_is_skip(int16_t attr_num){
    int16_t table[14]={5, //DELAY
        5, //PPDEL
        5, //TAPE
        5, //MULTITAP
        5, //CLOUD
        5, //SHIMMER
        5, //TRANSPOSE
        4, //REVERB
        4, //PEDAL
        6, //RINGMOD
        6, //MODDEMOD
        5, //CRUSH
        5, //BBD
        5, //RESEQ
    };
    return table[attr_num];
}
#define NUM_ATTRS 16
struct attr_parse {
    USR_IN type;
    char names[NUM_ATTRS][64];
    int16_t min_val[NUM_ATTRS];
    int16_t max_val[NUM_ATTRS];
    int16_t default_val[NUM_ATTRS];
    char display_type[NUM_ATTRS]; //s(hort) f(requency) i(nterval) t(ime)? d(B)
    int16_t num_attr;
};
///TODO: add messages for delay and crush; print messages
char creation_messages[NUM_OPTIONS][512]={
    "echo echo echo coco...",                           ///DELAY
    "performing black magic on the feedback matrix",    ///PPDEL
    "visting vintage shops and trying to look cool",    ///TAPE
    "accessing long forgotten quintuplet magic",        ///MULTITAP
    "spraying mist...",                                 ///CLOUD
    "sprinkling glitter...",                            ///SHIMMER
    "activating robospeak...",                          ///TRANSPOSE
    "tuning space",                                     ///REVERB
    "calling the piano tuner",                          ///PEDAL
    "I am a robot... an eighties robot...",             ///RINGMOD
    "trying out some rad makeup...",                    ///MODDEMOD
    "transatlantic connection incoming...",             ///CRUSH
    "so you've read the manual, i gather",              ///BBD
    "CWEJMAN lässt grüßen"
};
struct attr_parse attr_types[NUM_OPTIONS]={
    {DELAY, {"time", "feedback", "filter", "spread", "volume", "skip", "wet"},
        {1,   0,   10,   0,    -60,     0,   0},
        {100, 100, 100, 100,  12,       100, 100},
        {95, 30,  100, 0,     0,        0, 100},
        "ssfsdss", 5},
    {PPDEL, {"time", "feedback", "filter", "spread", "volume", "skip", "wet"},
        {1,   0,   10,   0,    -60,     0,   0},
        {100, 100, 100, 100,  12,       100, 100},
        {100, 80,  100, 50,     0,      0, 100},
        "ssfsdss", 5},
    {TAPE, {"time", "feedback", "filter", "spread", "volume", "skip", "wet"},
        {1,   0,   10,   0,    -60,     0,   0},
        {100, 100, 100, 100,  12,       100, 100},
        {100, 30,  60, 40,    0,        0, 100},
        "ssfsdss", 5},
    {MULTITAP, {"time", "feedback", "filter", "spread", "volume", "skip", "wet"},
        {1,   0,   10,   0,    -60,     0,   0},
        {100, 100, 100, 100,  12,       100, 100},
        {100, 50,  100, 100,  0,        0, 100},
        "ssfsdss", 5},
    {CLOUD, {"spread", "density", "grain length", "transpose", "volume", "skip", "reverse", "feedback"},
        {1,   1,   1,   1,   -60,       0,   0, 0},
        {100, 300, 100, 190,  12,       100, 1, 100},
        {100, 290,  70,  100,  0,       0, 1, 60},
        "sssidsss", 7},
    {SHIMMER, {"spread", "density", "grain length", "transpose", "volume", "skip", "reverse", "feedback"},
        {1,   1,   1,   1,   -60,       0,   0, 0},
        {100, 300, 100, 190,  12,       100, 1, 100},
        {50, 190,  70,  180,  0,        0, 0, 0},
        "sssidsss", 7},
    {TRANSPOSE, {"spread", "density", "grain length", "transpose", "volume", "skip", "reverse", "feedback"},
        {1,   1,   1,   1,   -60,       0,   0, 0},
        {100, 300, 100, 190,  12,       100, 1, 100},
        {100, 90,  70,  140,  0,        0, 0, 0},
        "sssidsss", 7},
    {REVERB, {"feedback", "filter", "modulation", "volume", "skip", "wet"},
        {0, 1, 0,  -60,         0,   0},
        {99, 100, 100, 12,      100, 100},
        {95, 60, 35,  0,        0, 100},
        "sfsdss", 4},
    {PEDAL, {"feedback", "filter", "modulation", "volume", "skip", "wet"},
        {0, 1, 0,  -60,         0,   0},
        {99, 100, 100,  12,     100, 100},
        {97, 95, 0, 0,          0, 100},
        "sfsdss", 4},
    {RINGMOD, {"frequency", "FM freq","FM index", "fold", "offset", "volume", "skip", "wet"},
        {1, 1, 0,    0,    0,  -60,          0,   0},
        {95, 100, 100,100, 25, 12,      100, 100},
        {85, 30, 50,   0,  0,    0,          0, 100},
        "ffsssdss", 6},
    {MODDEMOD, {"frequency", "FM freq","FM index", "fold", "offset", "volume", "skip", "wet"},
        {1, 1, 0,      0, 0,     -60,          0,   0},
        {95, 90, 100, 100, 25, 12,       100, 100},
        {60, 40, 0,   0, 0,     0,           0, 100},
        "ffsssdss", 6},
    {CRUSH, {"nyquist", "interpolate", "filter", "mu","volume", "skip", "wet"},
        {30, 0, 1, 0,  -60,     0,   0},
        {100, 2, 100, 10, 12,   100, 100},
        {80, 2, 100, 6, 0,      0, 100},
        "fsfsdss",5},
    {BBD, {"quality", "feedback", "filter", "mod amt", "mod speed","volume", "skip", "wet"},
        {20,  0,   1,   0, 0,  -60,     0,   0},
        {300, 100, 100, 100,100, 12,      100, 100},
        {100, 80,  70, 0, 0, 0,       0, 100},
        "ssfssdss", 5},
    {RESEQ, {"gain", "bandwidth", "tilt", "crunch", "volume", "skip", "wet"},
        {-50,  20,   0,   0, -60,     0,   0},
        {60, 400, 100, 2, 12,      100, 100},
        {10, 100,  50,  1, 0,       0, 100},
        "ssfsdss", 5}
};
void display_attr(int16_t *attr, int32_t index, USR_IN type, int32_t *display_loc){
    float display_freq, my_base = pow(20000.0, 0.01);
    int32_t display_index, i;
    float interval_convert_tab[256];
    char interval_to_text_tab[32][32]={"-2 oct", "-1.5 oct", "-1 oct", "- min 7", "- min 6", "- tri", "- 5th", "- min 3", "- maj 2", "- min 2", "uni", "maj 2", "maj 3", "tri", "5th", "nat 6", "nat 7", "maj 7", "+2 oct", "+2.5 oct"};
    interval_convert_tab[0]=0.25f; interval_convert_tab[1]=0.375f;
    for (i=2; i<11; i++)
        interval_convert_tab[i] = (8.0+i-2.0)/16.0;
    for (i= 11; i<19; i++)
        interval_convert_tab[i] = (i-2.0)/8.0;
    interval_convert_tab[19]=2.5f;
    switch(attr_types[type].display_type[index]){
        case 's':
            mvprintw(display_loc[0], display_loc[1]-9, "        %d             ", attr[index]);
            break;
        case 'f':
            display_freq = 1 + pow(my_base, attr[index]);
            mvprintw(display_loc[0], display_loc[1]-9, "        %.2fHz             ", display_freq);
            break;
        case 'i':
            ///the values stored in attr[index] are broken up into chunks of 10 vals. the beginning and end of each chunk are mapped to known intervals encoded above. in between we use an exponential fit
            display_index = attr[index]/10;
            if (attr[index]%10 == 0){
            mvprintw(display_loc[0], display_loc[1]- 5, "%.3f  |%s        ", interval_convert_tab[display_index], interval_to_text_tab[display_index]);
            }
            else {
                display_freq = interval_convert_tab[display_index]*pow((interval_convert_tab[display_index+1]/interval_convert_tab[display_index]), (attr[index]%10/10.0));
                if (attr[index]%10<6)
                    mvprintw(display_loc[0], display_loc[1]- 5, "%.3f  %s       ", display_freq, interval_to_text_tab[display_index]);
                else
                    mvprintw(display_loc[0], display_loc[1]- 5, "%.3f  %s       ", display_freq, interval_to_text_tab[display_index+1]);
            }
            break;
        case 'd':
            mvprintw(display_loc[0], display_loc[1]-9, "        %+ddB           ", attr[index]);
            break;
        default:
            break;
    }
}
void* input_handler(void* in){
    BSOUND* bsound = (BSOUND*) in;
    WINDOW* wnd = bsound->wnd; char* line; char single_char; char name[256];
    line = (char*) calloc(sizeof(char)*256, 1);
    OP_STACK* cursor = (OP_STACK*) malloc(sizeof(OP_STACK));
    int32_t max_y, max_x, single_int, i, offset = 0;
    bool refresh_flag = 0, repeat_flag = 0,  load = 0, delete_flag = 0;
    int16_t num_attr = 0; int16_t page;
    int16_t input_loc[2], info_loc[2];
    if (!wnd)
    wnd = initscr();
    keypad(wnd, TRUE);
    getmaxyx(wnd, max_y, max_x);
    offset = max_x/2;
    input_loc[0]=4; input_loc[1]=max_x/3;
    info_loc[0]=max_y/3; info_loc[1]= max_x/3;
    bsound->head->x = 7;
    bsound->head->y = max_y - 2;
    bsound->wnd = wnd;
    cursor = bsound->head;
    memset(name, '\0', 256); memset(line, '\0', 256);
    refresh(); cbreak(); noecho();
    mvprintw(info_loc[0], max_x/4,  "<BSOUND>  Copyright (C) <2021>  <Richard Schwennicke>");
    mvprintw(info_loc[0]+1, max_x/4,"This program comes with ABSOLUTELY NO WARRANTY.");
    mvprintw(info_loc[0]+2, max_x/4,"This is free software, and you are welcome to re-");
    mvprintw(info_loc[0]+3, max_x/4,"distribute it under certain conditions; for details");
    mvprintw(info_loc[0]+4, max_x/4,"see the LICENSE.md in the current directory.");
    mvprintw(info_loc[0] + 6, max_x/4, "press any key to continue");
    move(info_loc[0]+7, max_x/4); refresh();
    getch(); erase();
    mvprintw(info_loc[0], max_x/4,     "CHECK YOUR AUDIO LEVELS!");
    mvprintw(info_loc[0] + 2, max_x/4, "WARNING: WHEN USING OPCODES THAT HAVE NO INHERENT DELAY");
    mvprintw(info_loc[0] + 3, max_x/4, "UNCONTROLLED (AND UGLY) FEEDBACK MAY OCCUR!");
    mvprintw(info_loc[0] + 5, max_x/4, "press any key to continue");
    move(info_loc[0]+6 , max_x/4); refresh();
    getch(); erase();
    mvprintw(info_loc[0], info_loc[1], "Do you want to load a saved state?");
    mvprintw(info_loc[0] +1, info_loc[1], "YES   |||   NO");
    move(info_loc[0] + 1, info_loc[1] + 11); refresh();
    while (1){
        single_int = getch();
        single_char = (char) single_int;
        if (single_char == 'a' || single_int == KEY_LEFT){
            move(info_loc[0] +1, info_loc[1]-1);
            load = 1;
            refresh();
        }
        if(single_char == 'd' || single_int == KEY_RIGHT){
            move(info_loc[0] +1, info_loc[1] + 11);
            load = 0;
            refresh();
        }
        if (single_char == '\n'){
            refresh_flag = 1;
            break;
        }
    }
    if (load){
       cursor = load_st(bsound, input_loc);
        refresh_flag = 1;
    }
    //main loop
#define NUM_PAGES 2
        while (1){
            if (refresh_flag){
                erase();
                getmaxyx(wnd, max_y, max_x);
            if (bsound->num_ops == 0){
                mvprintw(max_y-1, max_x/3, "SIGNAL CHAIN EMPTY");
                mvprintw(info_loc[0], info_loc[1], "to add input type ':'");
                move(max_y-1, max_x/2);
            }
            else {
                //print info
                if (bsound->bypass_flag)
                    mvprintw(0, max_x-7, "BYPASS");
                if (bsound->playback_flag)
                mvprintw(1, max_x-9, "PLAYBACK");
                //convert to switch statement?
                if (page == 0){
                mvprintw(info_loc[0], info_loc[1], "to add input type ':'");
                mvprintw(info_loc[0]+1, info_loc[1], "for navigation use A and D");
                mvprintw(info_loc[0]+2, info_loc[1], "for editing use arrow keys ");
                mvprintw(info_loc[0]+4, info_loc[1], "to quit type 'q'");
                //print stack
                offset = display_stack(bsound, max_x, max_y);
                move(cursor->y, cursor->x + offset);
                }
                else if (page == 1){
                    if (!bsound->filter_bank_exists){
                        page = (page +1 ) % NUM_PAGES;
                        continue;
                    }
                    if (bsound->filter_bank_active)
                        mvprintw(info_loc[0]-1, info_loc[1], "FILTERBANK ACTIVE  ");
                    else
                        mvprintw(info_loc[0]-1, info_loc[1], "FILTERBANK BYPASSED");
                    mvprintw(info_loc[0], info_loc[1], "to add input type ':'");
                    mvprintw(info_loc[0]+1, info_loc[1], "for navigation use A and D");
                    mvprintw(info_loc[0]+2, info_loc[1], "for editing use arrow keys ");
                    mvprintw(info_loc[0]+4, info_loc[1], "to quit type 'q'");
                    int16_t x_spacing = max_x / 12;
                    for (i=0; i<10; i++){
                        mvprintw(max_y-1, (i+1)*x_spacing , "band %d", i);
                    }
                }

            }
                refresh_flag = 0;
                refresh();
            }
            single_int = getch(); single_char = (char) single_int;
            switch (bsound->statusFlags){
                  case paInputUnderflow:
                  mvprintw(0, 0, "Input underflow");
                  case paInputOverflow:
                  mvprintw(0, 0, "Input overflow");
                  case paOutputUnderflow:
                  mvprintw(0, 0, "Output underflow");
                  case paOutputOverflow:
                  mvprintw(0, 0, "Output overflow");
                  default:
                  refresh();
                  break;
            }
            //global controls
            if (single_char == '\n' && delete_flag){
                delete_item(bsound, cursor);
                cursor = bsound->head;
                delete_flag = 0;
                OP_STACK* item = bsound->head; i = 0;
                bsound->head->x = 7; bsound->head->y = max_y-2;
                while (i<bsound->num_ops){
                    if (item->next_op == NULL){
                        break;
                    }
                    item->next_op->y = item->y;
                    item->next_op->x = item->x + getlength(item) +4;
                    i++; item = item->next_op;
                }
                refresh_flag = 1;
            }
            else if (single_char == '\n' && page == 1){
                bsound->filter_bank_active = ! bsound->filter_bank_active;
                refresh_flag = 1;
                continue;
            }
            if (single_char == 'q'){
                bsound->quit_flag = 1;
                break;
            }
            else if (single_char == ' '){
                bsound->record_flag = true;
                bsound->playback_flag = 0;
                erase();
                mvprintw(info_loc[0], info_loc[1], "RECORDING");
                mvprintw(info_loc[0]+2, info_loc[1], "to exit hit space");
                move(max_y -1, info_loc[1]);
                refresh();
                nodelay(wnd, 1);
                int16_t counter = 0;
                single_char = 'b';
                while (bsound->record_flag && single_char != ' '){
                    usleep(30000); single_char = getch();
                    if (counter++ > 9)
                        counter = 0;
                    mvprintw(info_loc[0]+1, info_loc[1]-5+ counter/3, "     .     ");
                    move(max_y -1, info_loc[1]);
                    refresh();
                }
                bsound->record_flag = 0;
                bsound->playback_flag = 1;
                nodelay(wnd, 0); refresh_flag = 1;
                continue;
            }
            else if (single_char == 'p' && bsound->num_ops){
                bsound->playback_flag = !bsound->playback_flag;
                if (bsound->playback_flag)
                mvprintw(1, max_x-9, "PLAYBACK");
                else
                    mvprintw(1, max_x-9, "        ");
                move(cursor->y, cursor->x + offset); refresh();
                continue;
            }
            if (single_char == '\t'){
                page = (page + 1)%2;
                refresh_flag = 1;
                continue;
            }
            if ((single_char == 'b' || single_char == 'B') && bsound->num_ops){
                bsound->bypass_flag = !bsound->bypass_flag;
                if (bsound->bypass_flag)
                mvprintw(0, max_x-9, "BYPASS");
                else
                    mvprintw(0, max_x-9, "      ");
                move(cursor->y, cursor->x + offset); refresh();
                continue;
            }
            //per page navigation
            if (page == 0){
            if (single_char == 'a'  && bsound->num_ops){
                if (repeat_flag)
                    refresh_flag = 1;
                if (cursor->previous_op != NULL)
                    cursor = cursor->previous_op;
                move(cursor->y, cursor->x + offset);
                refresh();
            }
            if (single_char == 'd'  && bsound->num_ops){
                if (repeat_flag)
                    refresh_flag = 1;
                if (cursor->next_op != NULL)
                    cursor = cursor->next_op;
                move(cursor->y, cursor->x + offset);
                refresh();
            }
                if (single_int == KEY_SRIGHT && bsound->num_ops){
                    cursor->attr[num_attr]+=10;
                    if (cursor->attr[num_attr]>attr_types[cursor->type].max_val[num_attr])
                        cursor->attr[num_attr] = attr_types[cursor->type].max_val[num_attr];
                    mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", attr_types[cursor->type].names[num_attr]);
                    int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                    display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                    repeat_flag = 1;
                    move(cursor->y, cursor->x +offset);
                    refresh();
                }
                if (single_int == KEY_SLEFT && bsound->num_ops){
                    cursor->attr[num_attr]-=10;
                    if (cursor->attr[num_attr]<attr_types[cursor->type].min_val[num_attr])
                        cursor->attr[num_attr] = attr_types[cursor->type].min_val[num_attr];
                    mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", attr_types[cursor->type].names[num_attr]);
                    int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                    display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                    repeat_flag = 1;
                    move(cursor->y, cursor->x +offset);
                    refresh();
                }

            if (single_int == KEY_RIGHT && bsound->num_ops){
                if (cursor->attr[num_attr]<attr_types[cursor->type].max_val[num_attr])
                    cursor->attr[num_attr]++;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", attr_types[cursor->type].names[num_attr]);
                int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                repeat_flag = 1;
                move(cursor->y, cursor->x +offset);
                refresh();
            }
            if (single_int == KEY_LEFT && bsound->num_ops){
                if (cursor->attr[num_attr]>attr_types[cursor->type].min_val[num_attr])
                    cursor->attr[num_attr]--;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", attr_types[cursor->type].names[num_attr]);
                int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                repeat_flag = 1;
                move(cursor->y, cursor->x +offset);
                refresh();
            }
            if (single_int == KEY_UP && bsound->num_ops){
                if(num_attr< attr_types[cursor->type].num_attr)
                    num_attr++;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", attr_types[cursor->type].names[num_attr]);
                //mvprintw(cursor->y - 3, cursor->x  +offset, "  %d  ", cursor->attr[num_attr]);
                int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                move(cursor->y, cursor->x +offset); repeat_flag = 1;
                refresh();
            }
            if (single_int == KEY_DOWN && bsound->num_ops){
                if(num_attr)
                    num_attr--;
                mvprintw(cursor->y-5, cursor->x -2 +offset, "%s      ", attr_types[cursor->type].names[num_attr]);
                //mvprintw(cursor->y - 3, cursor->x+offset , "  %d  ", cursor->attr[num_attr]);
                int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                move(cursor->y, cursor->x +offset); repeat_flag = 1;
                refresh();
            }
            }
            else if (page == 1){
                if (single_char == 'a')
                    ; // do something
            }
            if (single_char == ':'){
                noecho();
                mvprintw(input_loc[0], input_loc[1], "INPUT: ");
                mvprintw(info_loc[0], info_loc[1], "      OPCODES:\n \t\tdelay\t\ttape\t\tpingpong\tmultitap");
                mvprintw(info_loc[0]+2, 0, "\t\tshimmer\t\tcloud \t\ttranspose\tringmod");
                mvprintw(info_loc[0]+3, 0, "\t\tcrush\t\treverb\t\tpedal\t\tmoddemod");
                mvprintw(info_loc[0]+4, info_loc[1], "                  ");
                mvprintw(info_loc[0]+5, info_loc[1], "'delete' to delete");
                mvprintw(info_loc[0]+6, info_loc[1], "'clear' to clear");
                mvprintw(info_loc[0]+7, info_loc[1], "'push' to push ");
                mvprintw(info_loc[0]+8, info_loc[1], "'manual' for manual ");
                mvprintw(info_loc[0]+9, info_loc[1], "'preferences' for preferences ");
                move(input_loc[0], input_loc[1]+8);
                refresh();
                char instring[256]; i = 0;
                int32_t chars_entered = 0, num_tabs;
                while (i<255){
                    single_char = getch(); single_int = (int32_t) single_char;
                    if (single_int == 127/*backspace*/){
                        if (i>0){
                            instring[--i]=' ';
                            chars_entered = 0;
                            erase();  int32_t j;
                            mvprintw(input_loc[0], input_loc[1], "INPUT: ");
                            mvprintw(info_loc[0], info_loc[1], "      OPCODES:\n \t\tdelay\t\ttape\t\tpingpong\tmultitap");
                            mvprintw(info_loc[0]+2, 0, "\t\tshimmer\t\tcloud \t\ttranspose\tringmod");
                            mvprintw(info_loc[0]+3, 0, "\t\tcrush\t\treverb\t\tpedal\t\tmoddemod");
                            mvprintw(info_loc[0]+4, info_loc[1], "                  ");
                            mvprintw(info_loc[0]+5, info_loc[1], "'delete' to delete");
                            mvprintw(info_loc[0]+6, info_loc[1], "'clear' to clear");
                            mvprintw(info_loc[0]+7, info_loc[1], "'push' to push ");
                            mvprintw(info_loc[0]+8, info_loc[1], "'manual' for manual ");
                            mvprintw(info_loc[0]+9, info_loc[1], "'preferences' for preferences ");
                            move(input_loc[0], input_loc[1]+8);
                            for (j=0; j<i; j++)
                                mvprintw(input_loc[0], input_loc[1]+8 +j, "%c", line[j]);
                            refresh();
                        }
                        continue;
                    }
                    else if (single_char == '\n')
                        break;
                    else if (single_char == '\t'){
                        if (chars_entered == 0){
                            num_tabs = 1;
                        chars_entered = i;
                        i = autocomplete(line, i, num_tabs); int32_t j;
                        for (j=0; j<i; j++)
                            mvprintw(input_loc[0], input_loc[1]+8 +j, "%c", line[j]);
                        refresh();
                        }
                        else {
                            int32_t j;
                            for (j=0; j<i; j++)
                                mvprintw(input_loc[0], input_loc[1]+8 +j, " ", line[j]);
                            i = autocomplete(line, chars_entered, ++num_tabs);
                            for (j=0; j<i; j++)
                                mvprintw(input_loc[0], input_loc[1]+8 +j, "%c", line[j]);
                            refresh();
                        }
                    }
                    else {
                        if ((isalpha(single_char) || single_char == ' ' )&& i <25){
                        line[i++]=single_char;
                        chars_entered = 0;
                        }
                    }
                            int32_t j;
                    for (j=0; j<i; j++){
                        mvprintw(input_loc[0], input_loc[1]+8 +j, "%c", line[j]);
                    }
                    refresh();

                }
               // wgetnstr(wnd, line, 255);
                COMMAND *usr_in = parse(line, 255);
                if (usr_in->type == RESEQ && bsound->filter_bank_exists){
                    erase(); noecho();
                    mvprintw(info_loc[0], info_loc[1], "ERROR! FILTERBANK already exists");
                    move(info_loc[0]+1, info_loc[1]);
                    refresh(); sleep(1); refresh_flag = 1;
                    continue;
                }
                if (usr_in->type == DELETE){
                    //erase, then draw chain
                    if (bsound->num_ops == 0){
                          usr_in->type = ERROR;
                          refresh_flag = 1;
                          continue;
                    }
                    erase(); display_stack(bsound, max_x, max_y);
                    mvprintw(info_loc[0], info_loc[1], "choose item to be deleted and hit enter");
                    move(cursor->y, cursor->x + offset);
                    refresh();
                    delete_flag = 1;
                    noecho();
                    continue;
                }
                else if (usr_in->type == CLEAR){
                    cursor = bsound->head;
                    while (bsound->num_ops>0){
                        delete_item(bsound, cursor);
                        cursor = bsound->head;
                        while(cursor->next_op)
                        cursor = cursor->next_op;
                    }
                    offset = max_x/2;
                    refresh_flag = 1; noecho(); continue;
                }
                else if (usr_in->type == PREFERENCES_MENU){
                    noecho();
                    display_preferences_menu(bsound, input_loc);
                    refresh_flag = 1; continue;
              } // this won't work on linux, so maybe conditionally compile...
                else if (usr_in->type == MANUAL){
                    int64_t loc_length = strlen(bsound->programm_loc)-6;
                    char* command = (char*)malloc(sizeof(char)*(loc_length+40));
                    char* temp_loc = (char*)malloc(sizeof(char)*loc_length+10);
                    memset(command, '\0', loc_length +40); memset(temp_loc, '\0', loc_length+10);
                    memcpy(temp_loc, bsound->programm_loc, loc_length);
                    strcat(temp_loc, "manual.pdf");
                    memcpy(command, "open -a Preview ", 16);
                    strcat(command, temp_loc);
                    system(command);
                    free(command);
                    free(temp_loc);
                    refresh_flag = 1;
                    continue;
                }
                else if (usr_in->type == ERROR){
                    erase(); noecho();
                    mvprintw(info_loc[0], info_loc[1], "ERROR! WHAT DID YOU MEAN?");
                    move(info_loc[0]+1, info_loc[1]);
                    refresh(); sleep(1); refresh_flag = 1;
                    continue;
                }
                if (offset<9){
                    refresh_flag = 1; noecho(); erase();
                    mvprintw(info_loc[0], max_x/3, "Sorry, no more space for another opcode!");
                    refresh(); sleep(1);
                    continue;
                }
                usr_in->bsound = bsound; usr_in->cursor = cursor;
                if (insert_op(bsound, usr_in)==0){
                    OP_STACK* item = bsound->head; i = 0;
                    cursor = bsound->head;
                    while (i<bsound->num_ops){
                        if (item->next_op == NULL){
                            break;
                        }
                        item->next_op->y = item->y;
                        item->next_op->x = item->x + getlength(item) +4;
                        i++; item = item->next_op;
                    }
                }
                else {
                      erase();
                      mvprintw(info_loc[0], info_loc[1], "ERROR creating opcode");
                      mvprintw(info_loc[0]+2, info_loc[1], "PLEASE REPORT THIS");
                      refresh(); sleep(3);
                }
                erase(); noecho(); mvprintw(info_loc[0], max_x/3, "%s", creation_messages[usr_in->type]);
                refresh(); sleep(1);
                refresh_flag = 1;
            }

        }
    erase();
    mvprintw(info_loc[0], info_loc[1], "Do you want to save the current state?");
    mvprintw(info_loc[0] +1, info_loc[1], "YES   |||   NO");
    move(info_loc[0] +1, info_loc[1] -1);
    load = 1;
    while (1){
        single_int = getch();
        single_char = (char) single_int;
        if (single_char == 'a' || single_int == KEY_LEFT){
            move(info_loc[0] +1, info_loc[1] -1);
            load = 1;
            refresh();
        }
        if(single_char == 'd' || single_int == KEY_RIGHT){
            move(info_loc[0] +1, info_loc[1] + 11);
            load = 0;
            refresh();
        }
        if (single_char == '\n'){
            break;
        }
    }
    if (load){
        save_st(bsound, input_loc);
    }
    endwin();
    return NULL;
}
void error_message(const char* message, BSOUND* bsound){
    mvwprintw(bsound->wnd, 0, 0, "%s", message);
    refresh();
}
struct user_types{
    USR_IN type;
    char name[256];
    int32_t name_length;
    void* (*init_opcode)(BSOUND* bsound, USR_IN type);
    void (*dealloc_opcode)(BSOUND* bsound, void* data);
    void (*opcode)(float* input, float* output, void* data, const int16_t* attr, const BSOUND* bsound);
};
struct user_types all_types[NUM_OPTIONS]={
    {DELAY, "delay", 5, init_delay, dealloc_delay, delay},
    {PPDEL, "pingpong", 8, init_delay, dealloc_delay, delay},
    {TAPE, "tape", 4, init_delay, dealloc_delay, delay},
    {MULTITAP, "multitap", 8, init_delay, dealloc_delay, delay},
    {CLOUD, "cloud", 5, init_partikkel, dealloc_partikkel, partikkel},
    {SHIMMER, "shimmer", 7, init_partikkel, dealloc_partikkel, partikkel},
    {TRANSPOSE, "transpose", 9, init_partikkel, dealloc_partikkel, partikkel},
    {REVERB, "reverb", 6, init_reson, dealloc_reson, reson},
    {PEDAL, "pedal", 5, init_reson, dealloc_reson, reson},
    {RINGMOD, "ringmod", 7, init_moddemod, dealloc_moddemod, moddemod},
    {MODDEMOD, "moddemod", 8, init_moddemod, dealloc_moddemod, moddemod},
    {CRUSH, "crush", 5, init_crush, dealloc_crush, crush},
    {BBD, "bbd", 3, init_bbd, dealloc_bbd, bbd},
    {RESEQ, "reseq", 5, init_reseq, dealloc_reseq, reseq},
    {DELETE, "delete", 6, NULL, NULL, NULL},
    {CLEAR, "clear", 5, NULL, NULL, NULL},
    {MANUAL, "manual", 6, NULL, NULL, NULL},
    {PREFERENCES_MENU, "preferences", 11, NULL, NULL, NULL }
};
int32_t autocomplete(char* instring, int32_t length, int16_t option){
    int32_t i = 0, matching = 0, leading_white_space = 0;
    bool push_flag = false;
    char* to_be_processed = instring;
    //ignore leading whitespace
    while (to_be_processed[0]==' ' && leading_white_space<length){
    to_be_processed = &instring[++leading_white_space];
      }
      length -= leading_white_space;
      // autocomplete on push as well
    if (length>5){
          if (strncmp(to_be_processed, "push ", 5)==0){
          to_be_processed = &to_be_processed[5];
          push_flag = true;
          length -= 5;
    }
   }
   i=0;
    while (i<NUM_OPTIONS){
        if (strncmp(to_be_processed, all_types[i].name, length)==0)
            matching++;
        i++;
    }
    if (matching != 0){
        if (option>matching)
            option = option%matching + 1;
        i = 0; int32_t j = 0;
        while (i<NUM_OPTIONS){
            if (strncmp(to_be_processed, all_types[i].name, length)==0){
                if (++j==option)
                    break;
            }
            i++;
        }
        memcpy(to_be_processed, all_types[i].name, all_types[i].name_length);
        if (!push_flag)
        return all_types[i].name_length + leading_white_space;
        else
        return all_types[i].name_length +5 + leading_white_space;
    }
    else
        if (!push_flag)
        return length + leading_white_space;
        else
        return length +5 + leading_white_space ;
}
COMMAND* parse(char* line, int32_t length){
    USR_IN* parsed_in; int32_t i = 0; bool success = 0;
    char* to_be_processed = line;
    //parsed_in = (USR_IN*)malloc(sizeof(USR_IN)*NUM_ARGUMENTS);
    COMMAND* out_command = (COMMAND*)malloc(sizeof(COMMAND));
    out_command->push_flag = 0;
    ///many leading whitespaces still confuses you
    while (line[i]==' ' && i<length){
          to_be_processed = &line[++i];
   }
    if (strncmp(to_be_processed, "push", 4)==0){
        out_command->push_flag = 1;
        to_be_processed = &to_be_processed[5];
    }
    i = 0;
    while (i<NUM_OPTIONS){
        if (strncmp(to_be_processed, all_types[i].name,  all_types[i].name_length)==0){
            success = 1; break;
        }
        i++;
    }
    if (success){
        out_command->operator = all_types[i].opcode;
        out_command->dealloc = all_types[i].dealloc_opcode;
        out_command->init = all_types[i].init_opcode;
        out_command->type = all_types[i].type;
    }
    else {
        out_command->type = ERROR;
    }
    return out_command;
}
char* getname(OP_STACK* item){
    return all_types[item->type].name;
}
int32_t getlength(OP_STACK* item){
    return all_types[item->type].name_length;
}
int32_t display_stack(BSOUND* bsound, int32_t max_x, int32_t max_y){
    OP_STACK* current = bsound->head; int32_t i = 0; int32_t offset = 0;
    while (i<bsound->num_ops){
        offset+=getlength(current) +4;
        current = current->next_op; i++;
    } //total length of chain
    offset += 9;
    current = bsound->head; i = 0; offset = (max_x-offset)/2;
    mvprintw(current->y, 0 + offset, "IN -->");
    while (i<bsound->num_ops){
    mvprintw(current->y, current->x + offset, "%s -->", getname(current));
        //this is ugly, but it gets the value of "skip"
    if (current->attr[which_attr_is_skip(current->type)] && current->next_op)
        mvprintw(current->y-1, current->x + offset +1, "->>");
    if (current->next_op)
        current = current->next_op;
        i++;
    }
    mvprintw(current->y, current->x +offset + getlength(current) +4, "OUT");
    return offset;
}
int32_t insert_op(BSOUND* bsound, COMMAND* command){
    OP_STACK* new = (OP_STACK*)malloc(sizeof(OP_STACK));
    new->next_op = new->previous_op = NULL;
    new->dealloc = command->dealloc;
    new->func = command->operator;
    new->func_st = command->init(bsound, command->type);
    new->attr = (int16_t*)calloc(sizeof(int16_t)*32, 32);
    int32_t i = 0;
    for (i=0; i<=attr_types[command->type].num_attr;i++){
        new->attr[i]=attr_types[command->type].default_val[i];
    }
    if (new->func_st == NULL){
        return -1;
    }
    new->type = command->type;
    pthread_mutex_lock(&bsound->mymutex);
    if (bsound->num_ops == 0){
        new->y = bsound->head->y;
        bsound->head = new;
        bsound->head->x = 7; //insize
        bsound->num_ops+=1;
        pthread_mutex_unlock(&bsound->mymutex);
        return 0;
    }
    else {
        if (command->cursor){
            if (command->push_flag){
                new->next_op = bsound->head;
                new->y = bsound->head->y;
                new->x = bsound->head->x;
                bsound->head->previous_op = new;
                new->previous_op = NULL;
                command->cursor = new;
                bsound->head = new;
                bsound->num_ops +=1;
                pthread_mutex_unlock(&bsound->mymutex);
                return 0;
            }
            new->previous_op = command->cursor;
            new->next_op = command->cursor->next_op;
            if (command->cursor->next_op)
                command->cursor->next_op->previous_op = new;
            command->cursor->next_op = new;
            bsound->num_ops+=1;
            pthread_mutex_unlock(&bsound->mymutex);
            return 0;
        }
        pthread_mutex_unlock(&bsound->mymutex);
        return -1;
    }
}
void delete_item(BSOUND* bsound, OP_STACK* cursor){
    if (bsound->num_ops>0){
    pthread_mutex_lock(&bsound->mymutex);
    if (bsound->num_ops == 1){
        bsound->num_ops--;
        pthread_mutex_unlock(&bsound->mymutex);
        bsound->head->dealloc(bsound, bsound->head->func_st);
    }
    else {
        bsound->num_ops--;
        if(cursor->previous_op)
            cursor->previous_op->next_op = cursor->next_op;
        else
            bsound->head = cursor->next_op;
        if(cursor->next_op){
        cursor->next_op->previous_op = cursor->previous_op;
        }
        pthread_mutex_unlock(&bsound->mymutex);
        cursor->dealloc(bsound, cursor->func_st);
    }
}
}
OP_STACK* load_st(BSOUND* bsound, int16_t* print_loc){
    erase();
    COMMAND* command = (COMMAND* )malloc(sizeof(COMMAND));
    OP_STACK* cursor = (OP_STACK*) malloc(sizeof(OP_STACK));
    int64_t loc_length = strlen(bsound->programm_loc)-6;
    char* save_loc = (char*) malloc(sizeof(char)*(20+loc_length));
    char* temp_loc = (char*) malloc(sizeof(char)*(20+loc_length));
    char save_slot[10], line[128], single_char; int32_t type, i, single_int;
    FILE *ftemp;
    memset(save_loc, '\0', loc_length +20);
    memset(save_slot, '\0', 10);
    memcpy(save_loc, bsound->programm_loc, loc_length);
    mvprintw(print_loc[0], print_loc[1], "Which slot do you want to load?");
    for (i=0; i<NUM_SAVE_SLOTS; i++){
        memset(temp_loc, '\0', loc_length+20);
        memcpy(temp_loc, bsound->programm_loc, loc_length);
        sprintf(save_slot, "%d", i);
        strcat(temp_loc, save_slot);
        ftemp = fopen(temp_loc, "r");
        mvprintw(print_loc[0]+3*i+1, print_loc[1], "%s", save_slots[i]);
        if (ftemp){
        fgets(line, 128, ftemp);
        mvprintw(print_loc[0]+3*i+2, print_loc[1] -10, "last saved: %s", line);
        fclose(ftemp);
        }
        else {
            mvprintw(print_loc[0]+2*i+2, print_loc[1] -10, "couldn't find: %s", temp_loc);
            mvprintw(print_loc[0]-1, print_loc[1], "%s", bsound->programm_loc);
        }
        }
        move(print_loc[0]+1, print_loc[1]);
        i = 0;
        refresh();
        while (1){
            single_int = getch(); single_char = (char) single_int;
            if (single_char == 's' || single_int == KEY_DOWN){
                if (i<(NUM_SAVE_SLOTS-1))
                    i++;
                move(print_loc[0]+3*i+1, print_loc[1]);
                refresh();
            }
            if (single_char == 'w' || single_int == KEY_UP){
                if (i>0)
                    i--;
                move(print_loc[0]+3*i+1, print_loc[1]);
                refresh();
            }
            if (single_char == '\n'){
                break;
            }
        }
        memset(temp_loc, '\0', loc_length+20);
        memcpy(temp_loc, bsound->programm_loc, loc_length);
        sprintf(save_slot, "%d", i); strcat(temp_loc, save_slot);
        ftemp = fopen(temp_loc, "r");
        if (ftemp == NULL){
            erase();
            mvprintw(print_loc[0], print_loc[1], "weird... %s isn't where I left her", save_slots[i]);
            refresh();
            sleep(1);
            return NULL;
        }
        fgets(line, 128, ftemp);
        while(fgets(line, 128, ftemp)){
            //TODO: complete
            sscanf(line, "%d", &type);
            command->operator  = all_types[type].opcode;
            command->dealloc   = all_types[type].dealloc_opcode;
            command->init      = all_types[type].init_opcode;
            command->type      = all_types[type].type;
            command->bsound    = bsound;
            command->push_flag = 0;
            i = 0;
            cursor = bsound->head;
            while (i<bsound->num_ops){
                i++;
                if (cursor->next_op)
                    cursor = cursor->next_op;
            }
            if (bsound->num_ops<2)
                cursor->next_op = NULL;
            command->cursor = cursor;
            if (insert_op(bsound, command)==0){
                OP_STACK* item = bsound->head; int32_t scan_val, j=0;
                while (item->next_op)
                    item=item->next_op;
                i = 0;
                do
                i++;
                        //space char
                while (line[i]!=32 && line[i]!='\n');
                while (line[i]!= '\n'){
                    sscanf(&line[i], "%d", &scan_val);
                    item->attr[j]=scan_val;
                    j++;
                    do  i++;
                    while (line[i]!=32 && line[i]!='\n');
                }
                i = 0; item = bsound->head;
                while (i<bsound->num_ops){
                    if (item->next_op == NULL){
                        break;
                    }
                    item->next_op->y = item->y;
                    item->next_op->x = item->x + getlength(item) +4;
                    i++; item = item->next_op;
                }
            }
        }
    fclose(ftemp);
    return cursor;
}
void save_st(BSOUND* bsound, int16_t* print_loc){
    erase();
    int64_t loc_length = strlen(bsound->programm_loc)-6;
    char* save_loc = (char*) malloc(sizeof(char)*(20+loc_length));
    char* temp_loc = (char*) malloc(sizeof(char)*(20+loc_length));
    char save_slot[10], line[128], single_char; int32_t i, single_int;
    FILE *ftemp;
    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    memset(save_loc, '\0', loc_length +20);
    memset(save_slot, '\0', 10);
    memcpy(save_loc, bsound->programm_loc, loc_length);
    mvprintw(print_loc[0], print_loc[1], "Which slot do you want to save to?");
    for (i=0; i<NUM_SAVE_SLOTS; i++){
        memset(temp_loc, '\0', loc_length+20);
        memcpy(temp_loc, bsound->programm_loc, loc_length);
        sprintf(save_slot, "%d", i);
        strcat(temp_loc, save_slot);
        ftemp = fopen(temp_loc, "r");
        mvprintw(print_loc[0]+3*i+1, print_loc[1], "%s", save_slots[i]);
        if (ftemp){
        fgets(line, 128, ftemp);
        mvprintw(print_loc[0]+3*i+2, print_loc[1] -10, "last saved: %s", line);
        fclose(ftemp);
        }
        else {
            mvprintw(print_loc[0]+2*i+2, print_loc[1] -10, "couldn't find: %s", temp_loc);
            mvprintw(print_loc[0]-1, print_loc[1], "%s", bsound->programm_loc);
        }
        }
        move(print_loc[0]+1, print_loc[1]);
        i = 0;
        refresh();
        while (1){
            single_int = getch(); single_char = (char) single_int;
            if (single_char == 's' || single_int == KEY_DOWN){
                if (i<(NUM_SAVE_SLOTS-1))
                    i++;
                move(print_loc[0]+3*i+1, print_loc[1]);
                refresh();
            }
            if (single_char == 'w' || single_int == KEY_UP){
                if (i>0)
                    i--;
                move(print_loc[0]+3*i+1, print_loc[1]);
                refresh();
            }
            if (single_char == '\n'){
                break;
            }
        }
        memset(temp_loc, '\0', loc_length+20);
        memcpy(temp_loc, bsound->programm_loc, loc_length);
        sprintf(save_slot, "%d", i); strcat(temp_loc, save_slot);
        ftemp = fopen(temp_loc, "w");
        if (ftemp == NULL){
            erase();
            mvprintw(print_loc[0], print_loc[1], "weird... %s isn't where I left her", save_slots[i]);
            refresh();
            sleep(1);
            goto cleanup;
        }
        strftime(line, 128, "%c", tm);
        fprintf(ftemp, "%s\n", line);
        while(bsound->num_ops){
            int32_t i;
            fprintf(ftemp, "%d", bsound->head->type);
            for (i=0; i<=attr_types[bsound->head->type].num_attr; i++)
            fprintf(ftemp, " %d", bsound->head->attr[i]);
            fprintf(ftemp, "\n");
            delete_item(bsound, bsound->head);

            }
    fclose(ftemp);
cleanup:
    free(save_loc); free (temp_loc);
}
void print_preferences_menu(BSOUND* bsound, int16_t* print_loc){
    erase();
    if (bsound->mono_input)
    mvprintw(print_loc[0], print_loc[1], "MONO AUDIO: ENABLED ");
    else
    mvprintw(print_loc[0], print_loc[1], "MONO AUDIO: DISABLED ");
    mvprintw(print_loc[0]+1, print_loc[1], "BUFFER SIZE: %d", bsound->requested_bufsize);
    if (bsound->crossfade_looping)
    mvprintw(print_loc[0]+2, print_loc[1], "CROSS FADE LOOP BOUNDARIES: ENABLED  ");
    else
    mvprintw(print_loc[0]+2, print_loc[1], "CROSS FADE LOOP BOUNDARIES: DISABLED ");
    mvprintw(print_loc[0]+4, print_loc[1], "CHOOSE AND HIT ENTER TO TOGGLE/CYCLE THROUGH SETTINGS");
    #ifdef USE_CALLBACK
    mvprintw(print_loc[0]+7, print_loc[1], "CALLBACK FUNCTION USING %.3f %% proportion of CPU", 100.0*Pa_GetStreamCpuLoad(bsound->stream_handle));
    mvprintw(print_loc[0]+8, print_loc[1], "type 'r' to refresh");
    #endif
    mvprintw(print_loc[0]+10, print_loc[1], "type 'q' to go back");
    refresh();
}

void display_preferences_menu(BSOUND* bsound, int16_t* print_loc){
    char single_char; int32_t single_int, option_selected, max_options = 2;
    print_preferences_menu(bsound, print_loc);
    move(print_loc[0], print_loc[1] - 1); option_selected = 0; refresh();
    while (1){
        single_int = getch(); single_char = (char) single_int;
        if (single_char == 's' || single_int == KEY_DOWN){
            if (option_selected < max_options)
                move(print_loc[0] + ++option_selected, print_loc[1]-1 );
            refresh();
        }
        if (single_char == 'r')
        print_preferences_menu(bsound, print_loc);
        else if (single_char == 'w' || single_int == KEY_UP){
            if (option_selected > 0)
                move (print_loc[0]+ --option_selected, print_loc[1]-1);
            refresh();
        }
        else if (single_char == '\n'){
            switch (option_selected) {
                case 0:
                    bsound->mono_input = !(bsound->mono_input);
                    if (bsound->mono_input)
                        mvprintw(print_loc[0], print_loc[1], "MONO AUDIO: ENABLED  ");
                    else
                        mvprintw(print_loc[0], print_loc[1], "MONO AUDIO: DISABLED");
                    move(print_loc[0], print_loc[1]-1);
                    refresh();
                    break;
                case 1:
                    erase();
                    #ifndef USE_CALLBACK
                    mvprintw(print_loc[0], print_loc[1], "CHANGING BUFFER SIZE WILL ONLY TAKE EFFECT AFTER RESTARTING BSOUND");
                    #else
                    mvprintw(print_loc[0], print_loc[1], "JUST A MOMENT");
                    #endif
                    refresh(); if (bsound->requested_bufsize < 2048)
                        bsound->requested_bufsize *= 2;
                    else
                        bsound->requested_bufsize = 64; sleep(1);
                    mvprintw(print_loc[0]+2, print_loc[1], "PRESS ANY KEY TO CONTINUE");
                     getch();

                    print_preferences_menu(bsound, print_loc);
                    move(print_loc[0] + option_selected, print_loc[1]-1);
                    refresh();
                    break;
               case 2:
                    bsound->crossfade_looping = !(bsound->crossfade_looping);
                    if (bsound->crossfade_looping){
                    erase();
                    mvprintw(print_loc[0], print_loc[1], "LOOP CROSSFADING ENABLED");
                    mvprintw(print_loc[0]+3, print_loc[1], "PRESS ANY KEY TO CONTINUE");
                    refresh();
                    getch();
                    print_preferences_menu(bsound, print_loc);
                    }
                    else{
                          mvprintw(print_loc[0]+2, print_loc[1], "CROSS FADE LOOP BOUNDARIES: DISABLED ");
                    }
                    move(print_loc[0] + option_selected, print_loc[1]-1);
                    refresh();
                    break;
                default:
                    erase(); mvprintw(print_loc[0], print_loc[1], "INPUT NOT RECOGNIZED"); sleep(1);
                    print_preferences_menu(bsound, print_loc);
                    option_selected = 0; move(print_loc[0], print_loc[1] -1); refresh();
                    break;
            }

        }
        else if (single_char == 'q')
            break;
    }

}
