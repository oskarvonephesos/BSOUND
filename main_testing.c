//
//  main_testing.c
//  BSOUND
//
//  Created by Richard Schwennicke on 1/25/21.
//
//*              :::NEW IN VERSION 1.2:::
//               :::MULTITAP DELAY:::
//               :::DELETE :::
//               :::RING MOD AND MODDEMOD:::
//               ???INSERT???
//                  TRANSFORM OPCODE
//                  CORRELATE OPCODE
//               ???LOG FILES
//               ???WET/DRY OPCODES???
//               ???REMEMBER STATES???
//               ???INPUT/OUTPUT MAPPING???
//TODO: SWITCH A B; first ternary command!!!
//TODO: make verbosity of stack_ops optional
///switch should have its own op_stacks and send audio to one or the other (based on random linsegs)
///difficulties arrise on dealloc
#ifndef _STDLIB
#define _STDLIB
#include <stdlib.h>
#include <stdio.h>
#endif

#include <pthread.h>
#include <string.h>
#include "data_types.h"
#include "programm state.h"
#include "opcodes.h"
#include "portaudio.h"
#include "input_handling.h"
#include "opcodes.h"
#include "stack_actions.h"
#include "util_opcodes.h"
#include "meta_ops.h"
#include "logger.h"


//TODO: do more exciting things with panning
/*OP_LIST list[] = {
    {init_delay,     dealloc_delay,     delay,     "DELAY"},     //DELAY
    {init_delay,     dealloc_delay,     delay,     "PINGPONG"},     //PPDEL
    {init_delay,     dealloc_delay,     delay,     "TAPE"},     //TAPE
    {init_delay,     dealloc_delay,     delay,     "MULTITAP"},     //MULTITAP
    {init_partikkel, dealloc_partikkel, partikkel, "CLOUD"}, //CLOUD
    {init_partikkel, dealloc_partikkel, partikkel, "SHIMMER"}, //SHIMMER
    {init_partikkel, dealloc_partikkel, partikkel, "TRANSPOSE"}, //TRANSPOSE
    {init_reson,     dealloc_reson,     reson,     "REVERB"},     //REVERB
    {init_reson,     dealloc_reson,     reson,     "PEDAL"},     //PEDAL
    {init_moddemod,  dealloc_moddemod,  moddemod,  "RINGMOD"},  //RINGMOD
    {init_moddemod,  dealloc_moddemod,  moddemod,  "MODDEMOD"},  //MODDEMOD
    {init_crush,     dealloc_crush,     crush,     "CRUSH"},     //CRUSH
    {init_noisegate, dealloc_noisegate, noisegate, "NOISE GATE"}  //NOISEGATE
};*/
/**  @deprecated:
 */
void replace(op_stack** head, void (*operator)(float* input, float* output, void* data, BSOUND* bsound), void * data){
    (*head)->func = operator;
    (*head)->func_st = data;

}

HIPASS_OPS* hipass_ops;
void write_audio(float** input, float** output, op_stack* head, BSOUND* bsound){
    int i;
    float* temp;
    float* outval = *output, *inval=*input;
    op_stack* current_op = head;
    if (bsound->num_ops == 0){
        for (i=0; i<bsound->bufsize*bsound->num_chans; i++){
            outval[i]=0.0f;
        }
    }
    else{
       // hipass(inval, bsound, hipass_ops);
        current_op = bsound->head;
    for (i= 0; i<bsound->num_ops;){
        current_op->func(inval, outval, current_op->func_st, current_op->attr, bsound);
    //    printf("writing audio\n");
        i++;
        if (current_op->next_op != NULL)
            current_op = current_op->next_op;
        if (i<bsound->num_ops){
            if (current_op->func != transform && current_op->func != dummy_delete){
            temp = inval;
            inval = outval;
            outval = temp;
        }
        }
        *(output) = outval;
        *(input) = inval;
       
    }
    }
}

#define TAB_LENGTH 512
void rms_init(BSOUND* bsound);
void write_sine(float* samplein, BSOUND* bsound, long counter, FILE* fp, float* sin_tab){
    int i, ii = 0, my_counter, index, j;
    long bufsize = bsound->bufsize * bsound->num_chans;
    short num_chans = bsound->num_chans;
    my_counter  = 4;//(counter/5)%20 + 1;
    for (j=0; j<num_chans; j++){
        ii = 0;
    for (i=j; i<bufsize; ){
        index = my_counter*ii++;
        if (index>=TAB_LENGTH){
            while(index>=TAB_LENGTH)
            {index-=TAB_LENGTH;}
        }
        samplein[i] = 0.5*sin_tab[index];
        i += num_chans;
    }
    }
    for (i=0; i<bufsize; i++)
        fwrite(&(samplein[i]), sizeof(float), 1, fp);
}
void write_out(float* sampleout, BSOUND* bsound, FILE* fp){
    int i;
    for (i=0; i<bsound->bufsize*bsound->num_chans; i++){
        fwrite(&(sampleout[i]), sizeof(float), 1, fp);
    }
}
int main(int argc, const char * argv[]) {
    BSOUND * bsound = init_bsound();
    COMMAND command = init_command();
    op_stack* head = init_head();
    bsound->head = head;
    PRINT_COMMAND* print_command = init_print_command(bsound);
    print_command->programm_location = argv[0];
    int i;
    long counter = 0, num_cycles = 1600;
    //buffers portaudio writes to
    float *samplein, *sampleout;
    float* sin_tab= (float*) malloc(sizeof(float)*TAB_LENGTH);
    char* write_in_loc, *write_out_loc;
    FILE *fin, *fout;
    int loc_length = strlen(argv[0]) - 6;
    write_in_loc = (char*)malloc(sizeof(char)* (loc_length + 16));
    write_out_loc = (char*)malloc(sizeof(char)* (loc_length + 16));
    memset(write_in_loc, '\0', loc_length + 16);
    memcpy(write_in_loc, argv[0], loc_length);
    memset(write_out_loc, '\0', loc_length + 16);
    memcpy(write_out_loc, argv[0], loc_length);
    strcat(write_in_loc, "audio_in");
    strcat(write_out_loc, "audio_out");
    fin = fopen(write_in_loc, "wb");
    if (fin == NULL){
        fprintf(stderr, "ERROR WRITING TO FILE\n");
        return 0;
    }
    fout = fopen(write_out_loc, "wb");
    if (fout == NULL){
        fprintf(stderr, "ERROR WRITING TO OUT FILE\n");
    }
    ///welcome text for new version
    init_log(argv[0]);
   
    bsound->num_chans = 2;
        
    hipass_ops = hipass_init(bsound);
    samplein = (float *)calloc(sizeof(float)*bsound->bufsize*bsound->num_chans, 1);
    sampleout = (float *)calloc(sizeof(float)*bsound->bufsize*bsound->num_chans, 1);
    load_state((void*) print_command);
    for (i=0; i<TAB_LENGTH; i++){
        sin_tab[i]=sin(i*MY_2_PI/TAB_LENGTH);
    }
    while(1){
                 //  err = Pa_ReadStream(handle, samplein, bsound->bufsize);
        write_sine(samplein, bsound, counter, fin, sin_tab);
        write_audio(&samplein, &sampleout, head, bsound);
        write_out(sampleout, bsound, fout);
        bsound->head->attr[3]*=0.99;
            if (counter++ > num_cycles)
                break;
        
            }
    save_state((void*)print_command);
    fprintf(stdout, "...\tstopping audio-stream\t");
    free(samplein);
    free(sampleout);
    fclose(fin);
    fclose(fout);
    //deallocate resources!!!
    free_op_stack(bsound->head, bsound);
    if (bsound->num_ops!=0){
        free(bsound->head);
    }
    free(bsound);
    fprintf(stdout, "...\tdeallocating resources\t");
    fprintf(stdout,"...\t quit\n\n");
    fflush(stdout);
    return 0;
}
