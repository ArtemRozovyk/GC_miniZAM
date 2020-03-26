//
// Created by hernouf on 19/03/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gc.h"
#include "fifo.h"
#include "domain_state.h"
#include "config.h"

extern mlvalue accu; 
extern mlvalue env;
extern unsigned int sp;

mlvalue * contextValue = NULL;

void clear_semispace(semi_space space){
    free(space->tas);
    space->alloc_pointer=0;
}


void realoc_semispaces(semi_space from_space, semi_space to_space){
    from_space->tas = malloc(sizeof(mlvalue) * from_space->capcity);
    mlvalue *tmp = malloc(sizeof(mlvalue) * to_space->capcity);
    memcpy(tmp, to_space->tas, sizeof(mlvalue) * to_space->alloc_pointer);
    to_space->tas = tmp;
}

void change_capacities(semi_space from_space, semi_space to_space){
    if(to_space->alloc_pointer*2 >= to_space->capcity)
    {
        from_space->capcity *= 1.5;
        to_space->capcity *= 1.5;
    }
    else
    {
        from_space->capcity /= 1.5;
        to_space->capcity /= 1.5;
    }
}

mlvalue copy_to_space(semi_space to_space, mlvalue addr){
    mlvalue resultat;
    ml_fifo fifo = new_ml_fifo();
    push_fifo(fifo, &resultat, addr);
    mlvalue* block_to_space = to_space->tas + to_space->alloc_pointer;

    while (!is_empty_fifo(fifo)) {
        context ctx = pop_fifo(fifo);
        if(Tag(ctx.val)==FWD_PTR_T)
        {
            *(ctx.place)=Field0(ctx.val);
        }
        else
        {
            if(Size(ctx.val)==0)
            {
                to_space->alloc_pointer += 2;
            }
            else
            {
                to_space->alloc_pointer += Size(ctx.val)+1;
            }

            block_to_space[0] = Hd_val(ctx.val);
            for (size_t  i = 0; i < Size(ctx.val); i++) {
                block_to_space[i+1] = Field(ctx.val,i);
            }

            Hd_val(ctx.val) = Val_hd(Make_header(Size(ctx.val),Color(ctx.val), FWD_PTR_T));
            Field0(ctx.val) = Val_ptr(block_to_space + 1);

            for (size_t  i = 0; i < Size(ctx.val); i++) {
                if(Is_block(block_to_space[i+1]))
                {
                    push_fifo(fifo, block_to_space+i+1,block_to_space[i+1]);
                }
            }

            *(ctx.place) = block_to_space + 1;
            block_to_space = to_space->tas + to_space->alloc_pointer;
        }
    }

    free_fifo(fifo);
    return resultat;
}

void stop_gc(){
    semi_space from_space = Caml_state->space[Caml_state->current_semispace];
    semi_space to_space = Caml_state->space[(Caml_state->current_semispace + 1) %2];
    change_capacities(from_space,to_space);
    clear_semispace(from_space);

    realoc_semispaces(from_space, to_space);

    Caml_state->current_semispace = (Caml_state->current_semispace + 1) % 2;
}

void start_gc(){
    semi_space to_space = Caml_state->space[(Caml_state->current_semispace + 1) %2];

    if(Is_block(accu))
    {
        accu = copy_to_space(to_space, accu);
    }

    /* Copie de racine dans env */
    env = copy_to_space(to_space, env);

    /* Copie des racines dans stack */
    int i = sp-1;
    while (i >= 0) {
        if(Is_block(Caml_state->stack[i]))
        {
            Caml_state->stack[i] = copy_to_space(to_space, Caml_state->stack[i]);
        }
        i--;
    }

    if(contextValue){
        if(Is_block(*contextValue)){
            *contextValue = copy_to_space(to_space, *contextValue);
        }
    }

    stop_gc();
}

