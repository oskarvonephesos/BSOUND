#ifndef types_h
#define types_h
#include <pthread.h>
#include <curses.h>
#include <stdbool.h>
#ifndef MYFLT
typedef double MYFLT;
#endif
#ifndef _USR_IN
#define _USR_IN
typedef enum {
    DELAY, // delay opcodes
    PPDEL,
    TAPE,
    MULTITAP,
    CLOUD, // partikkel opcodes
    SHIMMER,
    TRANSPOSE,
    REVERB, // resonator opcodes
    PEDAL,
    RINGMOD, //moddemod opcodes
    MODDEMOD,
    CRUSH,
    BBD,
    DELETE,
    MANUAL,
    CLEAR,
    PREFERENCES_MENU,
    NUM_OPTIONS,
    //manipulate stack
    EMPTY = -1,
    ERROR = -2
}USR_IN;
#endif
typedef struct _BSOUND{
    int num_ops;
    int sample_rate;
    long default_buffer_length;
    short num_chans;
    long bufsize;
    short num_caesar;
    long out_of_range;
    float hi_damp;
    struct _op_stack* head;
    bool quit_flag;
    bool modify_flag;
    const char* programm_loc;
    bool mono_input;
    bool pause_flag;
    bool bypass_flag;
    WINDOW* wnd;
    } BSOUND;
typedef struct _op_stack{
    //operator
    void (*func)(float* input, float* output, void* data, const short* attr, const BSOUND* bsound);
    //dealloc function
    void (*dealloc)(BSOUND* bsound, void* data);
    //function data
    void * func_st;
    struct _op_stack *next_op;
    //currently not in use
    struct _op_stack *previous_op;
    //continuous ids
    int func_id;
    //quick access to original type
    USR_IN type;
    int y;
    int x;
    short* attr;
    bool change_flag;
    bool delete_flag;
    pthread_mutex_t lock;
}op_stack;
typedef struct {
    op_stack** head;
    void (*operator)(float* input, float* output, void* data, const short* attr, const BSOUND* bsound);
    void* (*init)(BSOUND* bsound,  USR_IN type);
    void (*dealloc)(BSOUND* bsound, void* data);
    ///this is for delete case
    op_stack* to_delete;
    op_stack* cursor;
    BSOUND* bsound;
    USR_IN type;
    bool push_flag;
}COMMAND;
#endif /* types_h */
