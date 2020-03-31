//
// Created by hernouf on 19/03/2020.
//
#include <string.h>
#include "fifo.h"
#include "domain_state.h"
#include "alloc.h"
#include "lists.h"
#include "config.h"

extern mlvalue accu; 
extern mlvalue env;
extern unsigned int sp;

mlvalue * contextValue = NULL;


void show_pages(mlvalue *page);

int est_dans_space(semi_space space, mlvalue addr){
    return (space->tas <= Ptr_val(addr)) && (Ptr_val(addr) < space->tas + space->capcity);
}

void add_new_page(caml_domain_state *c_s) {
    mlvalue *page = malloc(sizeof(mlvalue) * (Page_size / sizeof(mlvalue) + 1));
    page[0] = Make_header(Page_size / sizeof(mlvalue), RED,0, PAGE_T);
    long l = page[0];

    Caml_state->page_list = pushHead(Ptr_val(page), Caml_state->page_list);
    c_s->free_list = pushHead(Ptr_val(page + 1), c_s->free_list);
    c_s->free_list_sz += 1;
}

mlvalue *find_first_fit(caml_domain_state *pState, size_t size) {
    ml_list current_fl_chain = pState->free_list;
    while (current_fl_chain && current_fl_chain->val) {
        size_t bloc_size= Size(current_fl_chain->val);
        mlvalue * block = current_fl_chain->val;
        if(Tag(Val_ptr(block))==PAGE_T && bloc_size > size){
            Field(block, size) =
                    Make_header(bloc_size - (size + 1), RED,0, PAGE_T);
            current_fl_chain->val=current_fl_chain->val+size+1;
            return block-1;
        }
        if(Tag(Val_ptr(block))==INTERN_PAGE_T){
            if(bloc_size==size){
                //remove from list and give up the pointer
                pState->free_list=remove_value_from_list(block,pState->free_list);
                pState->free_list_sz--;
                return block-1;
            }
            if(bloc_size > size && bloc_size-size>=2){
                //we need to have a place for the next header
                Field(block, size) =
                        Make_header(bloc_size - (size + 1), RED, 0, INTERN_PAGE_T);
                current_fl_chain->val=current_fl_chain->val+size+1;
                return block-1;
            }
        }

        current_fl_chain=current_fl_chain->next;
    }
    return NULL;
}

void show_pages(mlvalue *page) {
    //start with adress of first block
    printf(" [ ");
    int i = 1;
    do {


        char *tag = NULL;
        char *color = NULL;


        switch (Tag(Val_ptr(page))) {
            case ENV_T:
                tag = "ENV_T";
                break;
            case CLOSURE_T:
                tag = "CLOSURE_T";
                break;
            case BLOCK_T:
                tag = "BLOCK_T";
                break;
            case PAGE_T:
                tag = "PAGE_T";
                break;
            case
                INTERN_PAGE_T:
                tag = "INTERN_PAGE_T";
                break;
        }


        switch (Color(Val_ptr(page))) {
            case WHITE:
                color = "WHITE";
                break;
            case GRAY:
                color = "GRAY";
                break;
            case BLACK:
                color = "BLACK";
                break;
            case RED:
                color = "RED";
                break;
        }
        if (i % 9 == 0) {
            printf("\n");
        }
        printf("( %s,%s,%p,%lu )", tag, color, page, Size(Val_ptr(page)));
        i++;
        if (Tag(Val_ptr(page)) == PAGE_T) {
            break;
        }
        page = page + (Size(Val_ptr(page)) + 1);
    } while (1);

    printf("]");
}

uint64_t old_size = 100 * KB;
uint64_t curr_size = 0;

mlvalue * allocate_in_free_list(size_t size) {
    mlvalue *block_to_mark = NULL;
    /*if ((curr_size + size + 1) * 1.5 < old_size)
    {*/
        if (size * sizeof(mlvalue) > Page_size / 2) {
            block_to_mark = caml_alloc((size + 1) * sizeof(mlvalue));
            Caml_state->big_list = pushHead(block_to_mark + 1, Caml_state->big_list);
            Caml_state->big_list_size++;
        } else {
            block_to_mark = find_first_fit(Caml_state, size);
            if (!block_to_mark) {
                //block of sufficient size wasn't found,new page is needed.
                add_new_page(Caml_state);
                block_to_mark = find_first_fit(Caml_state, size);
            }
        }
        return  block_to_mark;
    /*}
    else
    {
        //TODO: GC
        //Comment obtenir la nouvelle courent size?
    }*/
}


mlvalue copy_to_space(semi_space  from_space, semi_space to_space, mlvalue addr, int mode){
    mlvalue resultat;
    ml_fifo fifo = new_ml_fifo();
    push_fifo(fifo, &resultat, addr);
    mlvalue* block_to_space = to_space->tas + to_space->alloc_pointer;
    while (!is_empty_fifo(fifo))
    {
        context ctx = pop_fifo(fifo);
        if (Tag(ctx.val) == FWD_PTR_T)
        {
            mlvalue *place = (mlvalue *) ctx.place;
            *place = Field0(ctx.val);
        }
        else if (Survecu(ctx.val) && mode)
        {
            mlvalue *block_to_mark = allocate_in_free_list(Size(ctx.val) );
            block_to_mark[0] = Hd_val(ctx.val);
            for (size_t i = 0; i < Size(ctx.val); i++)
            {
                block_to_mark[i + 1] = Field(ctx.val, i);
            }
            Hd_val(ctx.val) = Make_header(Size(ctx.val), Color(ctx.val), Survecu(ctx.val), FWD_PTR_T);
            Field0(ctx.val) = Val_ptr(block_to_mark + 1);

            for (size_t i = 0; i < Size(ctx.val); i++)
            {
                if (Is_block(block_to_mark[i + 1]) && est_dans_space(from_space, block_to_mark[i + 1]))
                {
                    push_fifo(fifo, block_to_mark + i + 1, block_to_mark[i + 1]);
                    push_fifo(Caml_state->remembered_set, block_to_mark+i+1, block_to_mark[i+1]);
                }
            }
            mlvalue *place = (mlvalue *) ctx.place;
            *place = Val_ptr(block_to_mark + 1);
        }
        else
        {
            header_t  te = Hd_val(ctx.val);
            if (Size(ctx.val) == 0)
            {
                to_space->alloc_pointer += 2;
            }
            else
            {
                to_space->alloc_pointer += Size(ctx.val) + 1;
            }
            int survecu = mode ? Survecu(ctx.val) + 1 : Survecu(ctx.val);
            block_to_space[0] = Val_hd(Make_header(Size(ctx.val), Color(ctx.val), survecu, Tag(ctx.val)));
            header_t heerr = Hd_val(block_to_space +1 );
            for (size_t i = 0; i < Size(ctx.val); i++)
            {
                block_to_space[i + 1] = Field(ctx.val, i);
            }

            Hd_val(ctx.val) = Make_header(Size(ctx.val), Color(ctx.val), Survecu(ctx.val), FWD_PTR_T);
            header_t rere = Hd_val(ctx.val);
            Field0(ctx.val) = Val_ptr(block_to_space + 1);

            for (size_t i = 0; i < Size(ctx.val); i++)
            {
                if (Is_block(block_to_space[i + 1]) && est_dans_space(from_space, block_to_space[i + 1]))
                {
                    push_fifo(fifo, block_to_space + i + 1, block_to_space[i + 1]);
                }
            }

                mlvalue *place = (mlvalue *) ctx.place;
                *place = Val_ptr(block_to_space + 1);
                block_to_space = to_space->tas + to_space->alloc_pointer;
        }
    }

    free_fifo(fifo);
    return resultat;
}




void copy_all_to_space(semi_space from_space, semi_space to_space, int mode){

    if(Is_block(accu) && est_dans_space(from_space, accu))
    {
        accu = copy_to_space(from_space, to_space, accu, mode);
    }



    /* Copie de racine dans env */
    if(est_dans_space(from_space, env))
        env = copy_to_space(from_space, to_space, env, mode);

    /* Copie des racines dans stack */
    int i = sp-1;
    while (i >= 0) {
        if(Is_block(Caml_state->stack[i]) && est_dans_space(from_space, Caml_state->stack[i]))
        {
            Caml_state->stack[i] = copy_to_space(from_space, to_space, Caml_state->stack[i], mode);
        }
        i--;
    }

    if(contextValue){
        if(Is_block(*contextValue) && contextValue){
            *contextValue = copy_to_space(from_space, to_space, *contextValue, mode);
        }
    }

    /*printf("pl1\n");
    ml_list curr2=Caml_state->page_list;
    while(curr2&&curr2->val){
        show_pages(curr2->val+1);
        printf(" |-| \n\n");
        curr2=curr2->next;
    }*/

    if(!is_empty_fifo(Caml_state->remembered_set)){
        ml_fifo_field curr = Caml_state->remembered_set->start;
        while(curr){
            mlvalue * place = curr->ctx.place;
            mlvalue res =  copy_to_space(from_space ,to_space, curr->ctx.val, mode);
            *place = res;
            curr->ctx.val = res;
            curr = curr->next;
        }
    }


}

void start_gc(){
    semi_space from_space = Caml_state->space[Caml_state->current_semispace];
    semi_space to_space = Caml_state->space[(Caml_state->current_semispace + 1) %2];

    copy_all_to_space(from_space, to_space, 1);

    if(to_space->alloc_pointer*2 >= to_space->capcity)
    {
        from_space->capcity *= 1.5;
    }
    else
    {
        from_space->capcity /= (from_space->capcity/1.5 <= Semi_space_min_limit/8) ? 1 : 1.5;
    }

    Caml_state->space[Caml_state->current_semispace] = new_semispace(from_space->capcity);
    free_semispace(from_space);

    semi_space to_space_final;
    if(to_space->alloc_pointer*2 >= to_space->capcity)
        to_space_final = new_semispace(to_space->capcity*1.5);
    else {
        to_space_final = (to_space->capcity/1.5 <= Semi_space_min_limit/8) ? new_semispace(to_space->capcity) : new_semispace(to_space->capcity / 1.5);
    }
    copy_all_to_space(to_space, to_space_final, 0);
    free_semispace(to_space);
    Caml_state->space[(Caml_state->current_semispace + 1) %2] = to_space_final;

    Caml_state->current_semispace = (Caml_state->current_semispace + 1) % 2;
}

