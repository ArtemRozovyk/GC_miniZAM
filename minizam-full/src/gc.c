//
// Created by hernouf on 19/03/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include "gc.h"
#include "config.h"

extern caml_domain_state* Caml_state;
extern mlvalue accu; 
extern mlvalue env;
extern unsigned int sp;

void clear_semispace(semi_space space){
    free(space->tas);
    space->tas = malloc(sizeof(mlvalue)*Semi_space_size);
    space->alloc_pointer=0;
}

void change_capacities(semi_space from_space, semi_space to_space){
    if(to_space->alloc_pointer*2>=to_space->capcity)
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
    if(Tag(addr)==FWD_PTR_T)
    {
        return  Field0(addr);
    }
    else
    {
        mlvalue* block_to_space = to_space->tas + to_space->alloc_pointer;
        size_t pes = Size(addr)+1;
        if(Size(addr)==0)
        {
            to_space->alloc_pointer += 2;
        }
        else
        {
            to_space->alloc_pointer += Size(addr)+1;
        }
        int64_t aos = Field0(addr);
        block_to_space[0] = Hd_val(addr);
        for (size_t  i = 0; i < Size(addr); i++) {
            block_to_space[i+1] = Field(addr,i);
        }

        Hd_val(addr) = Val_hd(Make_header(Size(addr),Color(addr), FWD_PTR_T));
        Field0(addr) = Val_ptr(block_to_space + 1);

        for (size_t  i = 0; i < Size(addr); i++) {
            if(Is_block(block_to_space[i+1]))
            {
                block_to_space[i+1] = copy_to_space(to_space, block_to_space[i+1]);
            }
        }
        return Val_ptr(block_to_space + 1);
    }
}

void start_gc(){
    semi_space from_space = Caml_state->space[Caml_state->current_semispace];
    semi_space to_space = Caml_state->space[(Caml_state->current_semispace + 1) %2];
    /* Copie de  racine eventuelle dans accu */
    if(Is_block(accu))
    {
        accu = copy_to_space(to_space, accu);
    }

    /* Copie de racine dans env */
    env = copy_to_space(to_space, env);

    /* Copie des racines dans stack */
    for (size_t i = sp-1 ; i != 0; i--) {
        if(Is_block(Caml_state->stack[i]))
        {
            Caml_state->stack[i] = copy_to_space(to_space, Caml_state->stack[i]);
        }
    }

    change_capacities(from_space,to_space);
    clear_semispace(from_space);
    Caml_state->current_semispace = (Caml_state->current_semispace + 1) % 2;
}

