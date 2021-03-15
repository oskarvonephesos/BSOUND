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
            if (bsound->statusFlags){
                  switch (bsound->statusFlags){
                  case paInputUnderflow:
                  mvprintw(0, 0, "Input underflow");
                  break;
                  case paInputOverflow:
                  mvprintw(0, 0, "Input overflow");
                  break;
                  case paOutputUnderflow:
                  mvprintw(0, 0, "Output underflow");
                  break;
                  case paOutputOverflow:
                  mvprintw(0, 0, "Output overflow");
                  break;
                  default:
                  break;
            }
            }
            refresh();
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
                single_char = '-';
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
                    if (cursor->attr[num_attr]>getmaxval(cursor, num_attr))
                        cursor->attr[num_attr] = getmaxval(cursor, num_attr);
                    mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", getdisplayname(cursor, num_attr));
                    int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                    display_attr(cursor->attr, num_attr, cursor->type, display_loc, getdisplaytype(cursor, num_attr));
                    repeat_flag = 1;
                    move(cursor->y, cursor->x +offset);
                    refresh();
                }
                if (single_int == KEY_SLEFT && bsound->num_ops){
                    cursor->attr[num_attr]-=10;
                    if (cursor->attr[num_attr]<getminval(cursor, num_attr))
                        cursor->attr[num_attr] = getminval(cursor, num_attr);
                    mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", getdisplayname(cursor, num_attr));
                    int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                    display_attr(cursor->attr, num_attr, cursor->type, display_loc, getdisplaytype(cursor, num_attr));
                    repeat_flag = 1;
                    move(cursor->y, cursor->x +offset);
                    refresh();
                }

            if (single_int == KEY_RIGHT && bsound->num_ops){
                if (cursor->attr[num_attr]<getmaxval(cursor, num_attr))
                    cursor->attr[num_attr]++;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", getdisplayname(cursor, num_attr));
                int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc, getdisplaytype(cursor, num_attr));
                repeat_flag = 1;
                move(cursor->y, cursor->x +offset);
                refresh();
            }
            if (single_int == KEY_LEFT && bsound->num_ops){
                if (cursor->attr[num_attr]>getminval(cursor, num_attr))
                    cursor->attr[num_attr]--;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", getdisplayname(cursor, num_attr));
                int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc, getdisplaytype(cursor, num_attr));
                repeat_flag = 1;
                move(cursor->y, cursor->x +offset);
                refresh();
            }
            if (single_int == KEY_UP && bsound->num_ops){
                if(num_attr< getnumdisplaytypes(cursor))
                    num_attr++;
                mvprintw(cursor->y-5, cursor->x -2  +offset, "%s        ", getdisplayname(cursor, num_attr));
                //mvprintw(cursor->y - 3, cursor->x  +offset, "  %d  ", cursor->attr[num_attr]);
                int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc, getdisplaytype(cursor, num_attr));
                move(cursor->y, cursor->x +offset); repeat_flag = 1;
                refresh();
            }
            if (single_int == KEY_DOWN && bsound->num_ops){
                if(num_attr)
                    num_attr--;
                mvprintw(cursor->y-5, cursor->x -2 +offset, "%s      ", getdisplayname(cursor, num_attr));
                //mvprintw(cursor->y - 3, cursor->x+offset , "  %d  ", cursor->attr[num_attr]);
                int32_t display_loc[2] = {cursor->y-3, cursor->x+offset};
                display_attr(cursor->attr, num_attr, cursor->type, display_loc, getdisplaytype(cursor, num_attr));
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
                else if (usr_in->type == LOAD_ST){
                      cursor = bsound->head;
                     while (bsound->num_ops>0){
                         delete_item(bsound, cursor);
                         cursor = bsound->head;
                         while(cursor->next_op)
                         cursor = cursor->next_op;
                     }
                     offset = max_x/2;
                      cursor = load_st(bsound, input_loc);
                      refresh_flag = 1; continue;
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
