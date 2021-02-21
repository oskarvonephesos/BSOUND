//
//  data_types.c
//  BSOUND
//
//  Created by Richard Schwennicke on 1/6/21.
//

#include "data_types.h"
#include "programm state.h"
RNGBUF* alloc_rngbuf( BSOUND* bsound, int length){
    short num_chans = bsound->num_chans;
    RNGBUF* buffer = (RNGBUF *) malloc(sizeof(RNGBUF));
    if (buffer == NULL)
        return NULL;
    int i;
    buffer->value = malloc (sizeof(MYFLT*)*num_chans);
    if (buffer->value == NULL)
    {
        //puts( "ERROR allocating memory");
        return NULL;
    }
    for(i=0; i<num_chans; i++){
    buffer->value[i] = (MYFLT*) calloc(sizeof(MYFLT)*length, 1);
        if (buffer->value[i] == NULL)
        {
           // puts( "ERROR allocating memory");
            return NULL;
        }
    }

    buffer->length = length;
    buffer->index = 0;
    buffer->available = 0;
    return buffer;
}
void dealloc_rngbuf(RNGBUF* buffer, BSOUND* bsound){
    int i;
    for (i=0; i<bsound->num_chans; i++){
    free(buffer->value[i]);
        buffer->value[i] = NULL;}
    free(buffer);
    buffer= NULL;
    
}
DELAY_LINE* alloc_delay_line(BSOUND* bsound, int length){
    DELAY_LINE* line = (DELAY_LINE*) malloc(sizeof(DELAY_LINE));
    if (line == NULL){
        //puts( "ERROR allocating memory \n");
        return NULL;
    }
    line->value = (MYFLT*) calloc (1, sizeof(MYFLT)*length);
    if (line->value == NULL){
       // puts( "ERROR allocating memory\n");
        return NULL;
    }
    line->length = length;
    line->index = 0;
    line->read_incr = 1.0f;
    line->read_index = 0.0f;
    line->prv_y0 = 0.0f;
    return line;
}
void dealloc_delay_line(DELAY_LINE* line, BSOUND* bsound){
    free (line->value);
    free(line);
    line = NULL;
}
