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
#include <unistd.h>
#include <stdbool.h>
#define IN_SIZE 7
#define NUM_ARGUMENTS 3
char save_slots[4][32]={"esja", "grettisgata", "harpa", "reykjadalur"};
#ifndef NUM_SAVE_SLOTS
#define NUM_SAVE_SLOTS 4
#endif
COMMAND* parse(char* line, int length);
int insert_op(BSOUND* bsound, COMMAND* command);
char* getname(op_stack* item);
int getlength(op_stack* item);
int display_stack(BSOUND* bsound, int max_x, int max_y);
void delete_item(BSOUND* bsound, op_stack* cursor);
op_stack* load_st(BSOUND* bsound, short* print_loc);
void save_st(BSOUND* bsound, short* print_loc);
void display_preferences_menu(BSOUND* bsound, short* print_loc);

short which_attr_is_skip(int attr_num){
    short table[13]={5, //DELAY
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
    };
    return table[attr_num];
}
#define NUM_ATTRS 16
struct attr_parse {
    USR_IN type;
    char names[NUM_ATTRS][64];
    short min_val[NUM_ATTRS];
    short max_val[NUM_ATTRS];
    short default_val[NUM_ATTRS];
    char display_type[NUM_ATTRS]; //s(hort) f(requency) i(nterval) t(ime)? d(B)
    short num_attr;
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
    {CLOUD, {"spread", "density", "grain length", "transpose", "volume", "skip", "wet"},
        {1,   1,   1,   1,   -60,       0,   0},
        {100, 300, 100, 190,  12,       100, 100},
        {100, 260,  70,  100,  0,       0, 100},
        "sssidss", 5},
    {SHIMMER, {"spread", "density", "grain length", "transpose", "volume", "skip", "wet"},
        {1,   1,   1,   1,   -60,       0,   0},
        {100, 300, 100, 190,  12,       100, 100},
        {50, 190,  70,  180,  0,        0, 100},
        "sssidss", 5},
    {TRANSPOSE, {"spread", "density", "grain length", "transpose", "volume", "skip", "wet"},
        {1,   1,   1,   1,   -60,       0,   0},
        {100, 300, 100, 190,  12,       100, 100},
        {100, 90,  70,  140,  0,        0, 100},
        "sssidss", 5},
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
    {BBD, {"time", "feedback", "filter", "crunch", "volume", "skip", "wet"},
        {20,  0,   1,   0, -60,     0,   0},
        {300, 100, 100, 2, 12,      100, 100},
        {100, 80,  70,  1, 0,       0, 100},
        "ssfsdss", 5}
};
void display_attr(short *attr, int index, USR_IN type, int *display_loc){
    float display_freq, my_base = pow(20000.0, 0.01);
    int display_index, i;
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
    WINDOW* wnd; char line[256]; char single_char; char name[256];
    op_stack* cursor = (op_stack*) malloc(sizeof(op_stack));
    int max_y, max_x, single_int, i, offset = 0;
    bool refresh_flag = 0, repeat_flag = 0,  load = 0, delete_flag = 0;
    short num_attr = 0;
    short input_loc[2], info_loc[2];
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
    while (1){
        single_int = getch();
        single_char = (char) single_int;
        if (single_char == 'a' || single_int == KEY_LEFT){
            move(info_loc[0] +1, info_loc[1]);
            load = 1;
            refresh();
        }
        if(single_char == 'd' || single_int == KEY_RIGHT){
            move(info_loc[0] +1, info_loc[1] + 9);
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
                mvprintw(info_loc[0], info_loc[1], "to add input type ':'");
                mvprintw(info_loc[0]+1, info_loc[1], "for navigation use A and D");
                mvprintw(info_loc[0]+2, info_loc[1], "for editing use arrow keys ");
                mvprintw(info_loc[0]+4, info_loc[1], "to quit type 'q'");
                //print stack
                offset = display_stack(bsound, max_x, max_y);
                move(cursor->y, cursor->x + offset);
            }
                refresh_flag = 0;
                refresh();
            }
            single_int = getch(); single_char = (char) single_int;
            if (single_char == '\n' && delete_flag){
                delete_item(bsound, cursor);
                cursor = bsound->head;
                delete_flag = 0;
                op_stack* item = bsound->head; i = 0;
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
            if (single_char == 'q'){
                bsound->quit_flag = 1;
                break;
            }
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
                    //mvprintw(cursor->y-3, cursor->x+offset, "  %d  ", cursor->attr[num_attr]);
                    int display_loc[2] = {cursor->y-3, cursor->x+offset};
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
                    //mvprintw(cursor->y-3, cursor->x+offset, "  %d  ", cursor->attr[num_attr]);
                    int display_loc[2] = {cursor->y-3, cursor->x+offset};
                    display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                    repeat_flag = 1;
                    move(cursor->y, cursor->x +offset);
                    refresh();
                }

            if (single_int == KEY_RIGHT && bsound->num_ops){
                if (cursor->attr[num_attr]<attr_types[cursor->type].max_val[num_attr])
                    cursor->attr[num_attr]++;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", attr_types[cursor->type].names[num_attr]);
                //mvprintw(cursor->y-3, cursor->x+offset, "  %d  ", cursor->attr[num_attr]);
                int display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                repeat_flag = 1;
                move(cursor->y, cursor->x +offset);
                refresh();
            }
            if (single_int == KEY_LEFT && bsound->num_ops){
                if (cursor->attr[num_attr]>attr_types[cursor->type].min_val[num_attr])
                    cursor->attr[num_attr]--;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", attr_types[cursor->type].names[num_attr]);
                //mvprintw(cursor->y-3, cursor->x+offset, "  %d  ", cursor->attr[num_attr]);
                int display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                repeat_flag = 1;
                move(cursor->y, cursor->x +offset);
                refresh();
            }
            if (single_int == KEY_UP){
                if(num_attr< attr_types[cursor->type].num_attr)
                    num_attr++;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", attr_types[cursor->type].names[num_attr]);
                //mvprintw(cursor->y - 3, cursor->x  +offset, "  %d  ", cursor->attr[num_attr]);
                int display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                move(cursor->y, cursor->x +offset); repeat_flag = 1;
                refresh();
            }
            if (single_int == KEY_DOWN){
                if(num_attr)
                    num_attr--;
                mvprintw(cursor->y-5, cursor->x -2 +offset, "%s      ", attr_types[cursor->type].names[num_attr]);
                //mvprintw(cursor->y - 3, cursor->x+offset , "  %d  ", cursor->attr[num_attr]);
                int display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc);
                move(cursor->y, cursor->x +offset); repeat_flag = 1;
                refresh();
            }
            if ((single_char == 'b' || single_char == 'B') && bsound->num_ops){
                bsound->bypass_flag = !bsound->bypass_flag;
                if (bsound->bypass_flag)
                mvprintw(0, max_x-7, "BYPASS");
                else
                    mvprintw(0, max_x-7, "      ");
                move(cursor->y, cursor->x + offset); refresh();
            }
            if (single_char == ':'){
                echo();
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
                wgetnstr(wnd, line, 255);
                COMMAND *usr_in = parse(line, 255);
                if (usr_in->type == DELETE){
                    //erase, then draw chain
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
                    int loc_length = strlen(bsound->programm_loc)-6;
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
                    op_stack* item = bsound->head; i = 0;
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
                erase(); noecho(); mvprintw(info_loc[0], max_x/3, "%s", creation_messages[usr_in->type]);
                refresh(); sleep(1);refresh_flag = 1;
            }

        }
    erase();
    mvprintw(info_loc[0], info_loc[1], "Do you want to save the current state?");
    mvprintw(info_loc[0] +1, info_loc[1], "YES   |||   NO");
    move(info_loc[0] +1, info_loc[1]);
    load = 1;
    while (1){
        single_int = getch();
        single_char = (char) single_int;
        if (single_char == 'a' || single_int == KEY_LEFT){
            move(info_loc[0] +1, info_loc[1]);
            load = 1;
            refresh();
        }
        if(single_char == 'd' || single_int == KEY_RIGHT){
            move(info_loc[0] +1, info_loc[1] + 12);
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
    int name_length;
    void* (*init_opcode)(BSOUND* bsound, USR_IN type);
    void (*dealloc_opcode)(BSOUND* bsound, void* data);
    void (*opcode)(float* input, float* output, void* data, const short* attr, const BSOUND* bsound);
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
    {DELETE, "delete", 6, NULL, NULL, NULL},
    {CLEAR, "clear", 5, NULL, NULL, NULL},
    {MANUAL, "manual", 3, NULL, NULL, NULL},
    {PREFERENCES_MENU, "preferences", 5, NULL, NULL, NULL }
};
COMMAND* parse(char* line, int length){
    USR_IN* parsed_in; int i = 0; bool success = 0;
    parsed_in = (USR_IN*)malloc(sizeof(USR_IN)*NUM_ARGUMENTS);
    COMMAND* out_command = (COMMAND*)malloc(sizeof(COMMAND));
    out_command->push_flag = 0;
    while (i<NUM_OPTIONS){
        if (strncmp(line, "push", 4)==0){
            out_command->push_flag = 1;
            line = &line[5];
        }
        if (strncmp(line, all_types[i].name,  all_types[i].name_length)==0){
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
char* getname(op_stack* item){
    return all_types[item->type].name;
}
int getlength(op_stack* item){
    return all_types[item->type].name_length;
}
int display_stack(BSOUND* bsound, int max_x, int max_y){
    op_stack* current = bsound->head; int i = 0; int offset = 0;
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
int insert_op(BSOUND* bsound, COMMAND* command){
    op_stack* new = (op_stack*)malloc(sizeof(op_stack));
    new->dealloc = command->dealloc;
    new->func = command->operator;
    new->func_st = command->init(bsound, command->type);
    new->attr = (short*)calloc(sizeof(short)*32, 32);
    int i = 0;
    for (i=0; i<=attr_types[command->type].num_attr;i++){
        new->attr[i]=attr_types[command->type].default_val[i];
    }
    if (new->func_st == NULL){
        return -1;
    }
    new->type = command->type;
    //mvprintw(0, 0, "new: %p; bsound->head %p num_ops %d", new, bsound->head, bsound->num_ops);
    refresh();
    if (bsound->num_ops == 0){
        new->y = bsound->head->y;
        bsound->head = new;
        bsound->head->next_op = NULL;
        bsound->head->previous_op = NULL;
        bsound->head->x = 7; //insize
        bsound->num_ops+=1;
       // mvprintw(1, 0, "new: %p; bsound->head %p num_ops %d", new, bsound->head, bsound->num_ops);
        refresh();
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
                return 0;
            }
            new->previous_op = command->cursor;
            new->next_op = command->cursor->next_op;
            if (command->cursor->next_op)
                command->cursor->next_op->previous_op = new;
            command->cursor->next_op = new;
            bsound->num_ops+=1;
            mvprintw(2, 0, "cursor: %p; bsound->head %p num_ops %d", command->cursor, bsound->head, bsound->num_ops);
            mvprintw(3, 0, "next: %p; previous: %p", new->next_op, new->previous_op);
            refresh();
            return 0;
        }
        return -1;
    }
}
void delete_item(BSOUND* bsound, op_stack* cursor){
    if (bsound->num_ops == 1){
        bsound->num_ops--;
        bsound->head->dealloc(bsound, bsound->head->func_st);
        //cursor = bsound->head;
    }
    else {
        bsound->num_ops--;
        if(cursor->previous_op)
            cursor->previous_op->next_op = cursor->next_op;
        else
            bsound->head = cursor->next_op;
        if(cursor->next_op)
        cursor->next_op->previous_op = cursor->previous_op;
        cursor->dealloc(bsound, cursor->func_st);
        //cursor = bsound->head;
    }
}
op_stack* load_st(BSOUND* bsound, short* print_loc){
    erase();
    COMMAND* command = (COMMAND* )malloc(sizeof(COMMAND));
    op_stack* cursor = (op_stack*) malloc(sizeof(op_stack));
    int loc_length = strlen(bsound->programm_loc)-6;
    char* save_loc = (char*) malloc(sizeof(char)*(20+loc_length));
    char* temp_loc = (char*) malloc(sizeof(char)*(20+loc_length));
    char save_slot[10], line[128], single_char; int type, i, single_int;
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
            command->operator = all_types[type].opcode;
            command->dealloc = all_types[type].dealloc_opcode;
            command->init = all_types[type].init_opcode;
            command->type = all_types[type].type;
            command->bsound = bsound;
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
                op_stack* item = bsound->head; int scan_val, j=0;
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
void save_st(BSOUND* bsound, short* print_loc){
    erase();
    int loc_length = strlen(bsound->programm_loc)-6;
    char* save_loc = (char*) malloc(sizeof(char)*(20+loc_length));
    char* temp_loc = (char*) malloc(sizeof(char)*(20+loc_length));
    char save_slot[10], line[128], single_char; int i, single_int;
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
            int i;
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
void print_preferences_menu(BSOUND* bsound, short* print_loc){
    erase();
    if (bsound->mono_input)
    mvprintw(print_loc[0], print_loc[1], "MONO AUDIO: ENABLED ");
    else
    mvprintw(print_loc[0], print_loc[1], "MONO AUDIO: DISABLED ");
    mvprintw(print_loc[0]+1, print_loc[1], "BUFFER SIZE: %d", bsound->bufsize);
    mvprintw(print_loc[0]+3, print_loc[1], "CHOOSE AND HIT ENTER TO TOGGLE/CYCLE THROUGH SETTINGS");
    mvprintw(print_loc[0]+4, print_loc[1], "type 'q' to go back");
    refresh();
}
void display_preferences_menu(BSOUND* bsound, short* print_loc){
    char single_char; int single_int, option_selected, max_options = 1;
    print_preferences_menu(bsound, print_loc);
    move(print_loc[0], print_loc[1] - 1); option_selected = 0; refresh();
    while (1){
        single_int = getch(); single_char = (char) single_int;
        if (single_char == 's' || single_int == KEY_DOWN){
            if (option_selected < max_options)
                move(print_loc[0] + ++option_selected, print_loc[1]-1 );
            refresh();
        }
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
                    mvprintw(print_loc[0], print_loc[1], "CHANGING BUFFER SIZE");
                    bsound->pause_flag = 1;
                    if (bsound->bufsize < 2048)
                        bsound->bufsize *= 2;
                    else
                        bsound->bufsize = 64;
                    sleep(1);
                    bsound->pause_flag = 0;
                    print_preferences_menu(bsound, print_loc);
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
