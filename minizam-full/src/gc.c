//
// Created by human on 24/03/2020.
//

#include <stdbool.h>
#include "lists.h"
#include "domain_state.h"
#include "config.h"


ml_list free_empty_pages(ml_list pMll);

void updateFreeList(mlvalue *pInt, mlvalue *pInt1, size_t sz, tag_t tag);

bool contains(ml_list pMll, mlvalue *pInt);

void traverse_greys(ml_list greys) {
    if (!greys || !greys->val)
        return;
    int j = 0;
    ml_list nw_greys = ml_empty_list();
    while (greys && greys->val) {
        j++;
        for (int i = 0; i < Size(Val_ptr(greys->val)); i++) {
            if (Is_block(Field(Val_ptr(greys->val), i))) {
                mlvalue m = Field(Val_ptr(greys->val), i);
                if (Color(Field(Val_ptr(greys->val), i)) == WHITE) {
                    Set_Color(Field(Val_ptr(greys->val), i), GRAY);
                    nw_greys = pushHead(Ptr_val(Field(Val_ptr(greys->val), i)), nw_greys);
                }
            }
        }
        Set_Color(Val_ptr(greys->val), BLACK);
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
    if (Is_block(accu)) {
        Set_Color(env, GRAY);
        greys = pushHead(Ptr_val(env), greys);
    }
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


void show_page(mlvalue *page) {
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

ml_list sweep_pages(ml_list pages) {
    //pages->val contains adress of first case of a page
    ml_list curr = pages;
    while (curr && curr->val) {
        mlvalue *block = curr->val + 1;
        mlvalue *block32 = curr->val + 1;
        long l = (*curr->val);

        long j = 0;
        while (1) {
           // printf("%ld\n",j);
            j++;
            size_t block_sz = Size(Val_ptr(block));
            if (Color(Val_ptr(block)) == WHITE) {
                mlvalue *next_block = block + (block_sz + 1);
                //coalescing
                long i = 0;
                while (Tag(next_block) != PAGE_T && Color(next_block) != BLACK) {
                    i++;
                    if (Tag(next_block) == INTERN_PAGE_T) {
                        //find in freelist and update
                        updateFreeList(block, next_block, block_sz, INTERN_PAGE_T);
                    }
                    block_sz += Size(Val_ptr(block + (block_sz + 1))) + 1;

                    next_block = block + (block_sz + 1);
                }
                //ignore empty block

                    tag_t next_tag = Tag(next_block);
                    color_t next_color = Color(next_block);

                    //3 cases black, red, red+PAGE_T
                    if (next_color == BLACK) {
                        //add it to free list as a new "kinda-page"

                        Hd_val(Val_ptr(block)) = Make_header(block_sz, RED, INTERN_PAGE_T);
                        //update if contains
                        if (!contains(Caml_state->free_list, Ptr_val(block))) {
                            Caml_state->free_list = pushHead(Ptr_val(block), Caml_state->free_list);
                            Caml_state->free_list_sz++;
                        }

                    }
                    if (next_tag == PAGE_T) {
                        //find it in freelist and change the pointer to the new one.
                        updateFreeList(block, next_block, block_sz, PAGE_T);
                    }
                if (Tag(Val_ptr(block)) == PAGE_T) {
                    curr = curr->next;
                    break;
                }
                Caml_state->curr_page_space_size-=block_sz;
            }
            if (Tag(Val_ptr(block)) == PAGE_T) {
                curr = curr->next;
                break;
            }
            block = block + block_sz + 1;
            block32 = block + block_sz + 1;
        }

    }


    return pages;
}


bool contains(ml_list pMll, mlvalue *pInt) {
    while (pMll && pMll->val) {
        if (pMll->val == pInt) {
            return 1;
        }
        pMll = pMll->next;
    }
    return 0;
}

//find chain with pointer *next_block, change it with value of *block (that has its header updated)
void updateFreeList(mlvalue *block, mlvalue *next_block, size_t block_sz, tag_t tag) {
    ml_list currfl = Caml_state->free_list;
    mlvalue **old_p = NULL;
    while (currfl && currfl->val) {
        if (currfl->val == next_block) {
            old_p = &currfl->val;
        }
        currfl = currfl->next;
    }
    Hd_val(Val_ptr(block)) = Make_header(block_sz + (Size(Val_ptr(next_block))==0?1:Size(Val_ptr(next_block))), RED, tag);
    (*old_p) = block;
}


ml_list sweep(ml_list lst) {
    if (lst->val == NULL) {
        return lst;
    }
    while (lst != NULL && Color(lst->val) == WHITE) {
        ml_list tofree = lst;
        lst = lst->next;
        free((mlvalue *) tofree->val - 1);
        Caml_state->curr_page_space_size-=Size(tofree->val);

        free(tofree);
    }


    ml_list pred = lst;
    while (pred != NULL && pred->next != NULL) {
        while (pred->next != NULL) {
            if (Color(pred->next->val) == WHITE) {
                ml_list tofree = pred->next;
                pred->next = pred->next->next;
                free((mlvalue *) tofree->val - 1);
                Caml_state->curr_page_space_size-=Size(tofree->val);
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
    Caml_state->page_list=free_empty_pages(Caml_state->page_list);

}

ml_list free_empty_pages(ml_list lst) {
    if (lst->val == NULL) {
        return lst;
    }
    while (lst != NULL && (Size(lst->val+1) == Page_size / sizeof(mlvalue))) {
        ml_list tofree = lst;
        lst = lst->next;
        free((mlvalue *) tofree->val );
        free(tofree);
    }
    ml_list pred = lst;
    while (pred != NULL && pred->next != NULL) {
        while (pred->next != NULL) {
            if (Size(pred->next->val+1) == Page_size / sizeof(mlvalue)) {
                ml_list tofree = pred->next;
                pred->next = pred->next->next;
                free( tofree->val );
                free(tofree);
            }
            pred = pred->next;
        }
    }
    return lst;
}


void clear_state(){
    sweep(Caml_state->big_list);
    sweep_pages(Caml_state->page_list);
    ml_list curr_page = Caml_state->page_list;
    while (curr_page&&curr_page->val){
        free(curr_page->val);
        curr_page=curr_page->next;
    }
    release(Caml_state->free_list);
    release(Caml_state->page_list);
    release(Caml_state->big_list);
    free(Caml_state->free_list);
    free(Caml_state->page_list);
    free(Caml_state->big_list);
}