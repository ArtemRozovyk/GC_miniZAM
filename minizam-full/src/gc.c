//
// Created by human on 24/03/2020.
//

#include "lists.h"




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

void mark(mlvalue *stack, unsigned int curr_stack_sz,mlvalue accu,mlvalue env) {
    ml_list greys = ml_empty_list();
    if (Is_block(accu)){
        Set_Color(accu, GRAY);
        greys = pushHead(Ptr_val(accu), greys);
    }
    Set_Color(env, GRAY);
    greys = pushHead(Ptr_val(env),greys);
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




ml_list sweep(ml_list lst) {
    if (lst->val == NULL) {
        return lst;
    }
    while (lst != NULL && Color(lst->val) == WHITE) {
        ml_list tofree = lst;
        lst = lst->next;
        free((mlvalue * ) tofree->val-1);

        free(tofree);
    }


    ml_list pred = lst;
    while (pred != NULL && pred->next != NULL) {



        while (pred->next != NULL) {
            if(Color(pred->next->val) == WHITE ){
                ml_list tofree = pred->next;
                pred->next = pred->next->next;
                free((mlvalue * ) tofree->val-1);
                free(tofree);
            }
            pred = pred->next;

        }
    }
    ml_list reset_white=lst;
    while(reset_white&&reset_white->val){
        Set_Color(reset_white->val, WHITE);
        reset_white=reset_white->next;
    }
    return lst;
}
