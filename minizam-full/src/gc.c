//
// Created by hernouf on 19/03/2020.
//
#include <string.h>
#include "fifo.h"
#include "domain_state.h"
#include "alloc.h"

extern mlvalue accu; 
extern mlvalue env;
extern unsigned int sp;

mlvalue * contextValue = NULL;


int est_dans_space(semi_space space, mlvalue addr){
    return (space->tas <= Ptr_val(addr)) && (Ptr_val(addr) < space->tas + space->capcity);
}

mlvalue copy_to_space(semi_space  from_space, semi_space to_space, mlvalue addr){
    mlvalue resultat;
    ml_fifo fifo = new_ml_fifo();
    push_fifo(fifo, &resultat, addr);
    mlvalue* block_to_space = to_space->tas + to_space->alloc_pointer;
    while (!is_empty_fifo(fifo)) {
        context ctx = pop_fifo(fifo);
        if (Tag(ctx.val) == FWD_PTR_T) {
            mlvalue *place = (mlvalue *) ctx.place;
            *place = Field0(ctx.val);
        } else {
            if (Survie(ctx.val)) {
                int size = (Size(ctx.val) == 0) ? 1 : (Size(ctx.val));
                mlvalue *block_to_mark = NULL;
                if (size * sizeof(mlvalue) > Page_size / 2) {
                    block_to_mark = caml_alloc((size + 1) * sizeof(mlvalue));
                    Caml_state->big_list = pushHead(block + 1, Caml_state->big_list);
                    Caml_state->big_list_size++;
                } else {
                    block_to_mark = find_first_fit(Caml_state, size);
                    if (!block_to_mark) {
                        //block of sufficient size wasn't found,new page is needed.
                        add_new_page(Caml_state);
                        block_to_mark = find_first_fit(Caml_state, size);
                    }
                    //block has been found
                    block_to_mark[0] = Hd_val(ctx.val);
                    for (size_t i = 0; i < Size(ctx.val); i++) {
                        block_to_mark[i + 1] = Field(ctx.val, i);
                    }

                    Hd_val(ctx.val) = Make_header(Size(ctx.val), Color(ctx.val), Survie(ctx.val), FWD_PTR_T);
                    Field0(ctx.val) = Val_ptr(block_to_space + 1);
                }
            } else {
                if (Size(ctx.val) == 0) {
                    to_space->alloc_pointer += 2;
                } else {
                    to_space->alloc_pointer += Size(ctx.val) + 1;
                }

                block_to_space[0] = Make_header(Size(ctx.val), Color(ctx.val), Survie(ctx.val) + 1, Tag(ctx.val));
                for (size_t i = 0; i < Size(ctx.val); i++) {
                    block_to_space[i + 1] = Field(ctx.val, i);
                }

                Hd_val(ctx.val) = Make_header(Size(ctx.val), Color(ctx.val), Survie(ctx.val), FWD_PTR_T);
                Field0(ctx.val) = Val_ptr(block_to_space + 1);

                for (size_t i = 0; i < Size(ctx.val); i++) {
                    if (Is_block(block_to_space[i + 1])) {
                        push_fifo(fifo, block_to_space + i + 1, block_to_space[i + 1]);
                    }
                }

                mlvalue *place = (mlvalue *) ctx.place;
                *place = Val_ptr(block_to_space + 1);
                block_to_space = to_space->tas + to_space->alloc_pointer;
            }
        }
    }

    free_fifo(fifo);
    return resultat;
}

void copy_all_to_space(semi_space from_space, semi_space to_space){
    if(Is_block(accu))
    {
        accu = copy_to_space(from_space, to_space, accu);
    }

    /* Copie de racine dans env */
    env = copy_to_space(from_space, to_space, env);

    /* Copie des racines dans stack */
    int i = sp-1;
    while (i >= 0) {
        if(Is_block(Caml_state->stack[i]))
        {
            Caml_state->stack[i] = copy_to_space(from_space, to_space, Caml_state->stack[i]);
        }
        i--;
    }

    if(contextValue){
        if(Is_block(*contextValue)){
            *contextValue = copy_to_space(from_space, to_space, *contextValue);
        }
    }
}

void start_gc(){
    semi_space from_space = Caml_state->space[Caml_state->current_semispace];
    semi_space to_space = Caml_state->space[(Caml_state->current_semispace + 1) %2];

    copy_all_to_space(from_space, to_space);

    if(to_space->alloc_pointer*2 >= to_space->capcity)
    {
        from_space->capcity *= 1.5;
    }
    else
    {
        from_space->capcity /= 1.5;
    }

    Caml_state->space[Caml_state->current_semispace] = new_semispace(from_space->capcity);
    free_semispace(from_space);

    semi_space to_space_final;
    if(to_space->alloc_pointer*2 >= to_space->capcity)
        to_space_final = new_semispace(to_space->capcity*1.5);
    else
        to_space_final = new_semispace(to_space->capcity/1.5);

    copy_all_to_space(to_space, to_space_final);

    free_semispace(to_space);
    Caml_state->space[(Caml_state->current_semispace + 1) %2] = to_space_final;

    Caml_state->current_semispace = (Caml_state->current_semispace + 1) % 2;
}

