//
// Created by human on 24/03/2020.
//

#include "lists.h"
#include "domain_state.h"


void traverse_greys(ml_list greys) {
    if (!greys || !greys->val)
        return;
    ml_list nw_greys = ml_empty_list();
    while (greys && greys->val) {
        for (int i = 1; i < Size(greys->val); i++) {
            if (Is_block(Field(greys->val, i))) {
                if (Color(Field(greys->val, i)) == WHITE) {
                    Set_Color(Field(greys->val, i), GRAY);
                    nw_greys = pushHead(Ptr_val(Field(greys->val, i)), nw_greys);
                }
            }
        }
        Set_Color(greys->val, BLACK);
        greys = greys->next;
    }
    //show(nw_greys);
    //printf("\n");
    traverse_greys(nw_greys);
    release(nw_greys);
    free(nw_greys);


}

void mark(mlvalue *stack, unsigned int curr_stack_sz, mlvalue accu, mlvalue env) {
    ml_list greys = ml_empty_list();
    if (Is_block(accu)) {
        Set_Color(accu, GRAY);
        greys = pushHead(Ptr_val(accu), greys);
    }
    Set_Color(env, GRAY);
    greys = pushHead(Ptr_val(env), greys);
    for (int i = 0; i < curr_stack_sz; i++) {
        mlvalue v = stack[i];
        if (Is_block(stack[i])) {
            if (Color(stack[i]) == WHITE) {
                Set_Color(stack[i], GRAY);
                greys = pushHead(Ptr_val(stack[i]), greys);
            }
        }
    }
    traverse_greys(greys);
    //show(greys);
    release(greys);
    free(greys);
}


ml_list sweep_pages(ml_list pages) {


    //pages->val contains adress of first case of a page
    ml_list curr = pages;
    while (curr && curr->val) {
        mlvalue *block = pages->val + 1;
        long l = (*pages->val);
        size_t block_sz = Size(Val_ptr(block));

        if (Tag(Val_ptr(block)) == PAGE_T) {
            //consider next page
            curr = curr->next;
            continue;
        }

        do {

        if (Color(Val_ptr(block)) == WHITE) {

            while (Color(Val_ptr(block + (block_sz + 1))) == WHITE) {
                block_sz+=Size(Val_ptr(block + (block_sz + 1)))+1;
            }
            Hd_val(Val_ptr(block))=Make_header(block_sz, RED, Tag(Val_ptr(block)));

            Caml_state->free_list=pushHead(Ptr_val(block),Caml_state->free_list);
            Caml_state->free_list_sz++;

        }else{
            block=block+block_sz+1;
        }

        }while(Tag(Val_ptr(block)) != PAGE_T);





        curr = curr->next;
    }



return pages;
}


ml_list sweep_pages2(ml_list pages) {
    //pages->val contains adress of first case of a page
    ml_list curr = pages;
    while (pages && pages->val) {
        mlvalue *block = pages->val + 1;
        long l = Color(Val_ptr(block));
        size_t bl_sz = Size(Val_ptr(block));
        while (Tag(Ptr_val(block)) != PAGE_T) {
            if (Color(Val_ptr(block)) == WHITE) {
                //coalescing
                while (Color(Val_ptr(block + (bl_sz + 1))) == WHITE) {
                    bl_sz += Size(Val_ptr(block + (bl_sz + 1))) + 1;
                    if (Color(Val_ptr(block + (bl_sz + 1))) == RED) {
                        //found the end of the block inside a page
                        bl_sz++;
                    }
                }
                *(block - 1) = Make_header(bl_sz, WHITE, Tag(Val_ptr(block)));
                Caml_state->free_list = pushHead(block, Caml_state->free_list);
                Caml_state->free_list_sz++;
            }
            block = block + bl_sz + 1;
        }
        curr = curr->next;
    }
    return pages;
}

ml_list sweep(ml_list lst) {
    if (lst->val == NULL) {
        return lst;
    }
    while (lst != NULL && Color(lst->val) == WHITE) {
        ml_list tofree = lst;
        lst = lst->next;
        free((mlvalue *) tofree->val - 1);

        free(tofree);
    }


    ml_list pred = lst;
    while (pred != NULL && pred->next != NULL) {


        while (pred->next != NULL) {
            if (Color(pred->next->val) == WHITE) {
                ml_list tofree = pred->next;
                pred->next = pred->next->next;
                free((mlvalue *) tofree->val - 1);
                free(tofree);
            }
            pred = pred->next;

        }
    }
    ml_list reset_white = lst;
    while (reset_white && reset_white->val) {
        Set_Color(reset_white->val, WHITE);
        reset_white = reset_white->next;
    }
    return lst;
}

void gc() {
    sweep(Caml_state->big_list);
    sweep_pages(Caml_state->page_list);
}