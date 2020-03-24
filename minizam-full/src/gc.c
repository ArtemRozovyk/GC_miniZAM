//
// Created by human on 24/03/2020.
//

#include "lists.h"


void change_color(mlvalue *mlvalue1, color_t color) {
    header_t h = Hd_val((*mlvalue1));
    header_t newh = Make_header(Size((*mlvalue1)), color, Tag((*mlvalue1)));
    Hd_val((*mlvalue1)) = newh;
}


void traverse_greys(ml_list greys) {
    if (!greys || !greys->val)
        return;
    ml_list nw_greys = ml_empty_list();
    while (greys && greys->val) {
        for (int i = 1; i < Size(*greys->val); i++) {
            if (Is_block(Field(*greys->val, i))) {
                if (Color(Field(*greys->val, i)) == WHITE) {
                    change_color(&Field(*greys->val, i), GRAY);
                    nw_greys = pushHead(&Field(*greys->val, i), nw_greys);
                }
            }
        }
        change_color(greys->val, BLACK);
        greys = greys->next;
    }
    //show(nw_greys);
    //printf("\n");
    traverse_greys(nw_greys);


}

void mark(mlvalue *stack, unsigned int curr_stack_sz) {
    ml_list greys = ml_empty_list();
    for (int i = 0; i < curr_stack_sz; i++) {
        mlvalue v = stack[i];
        if (Is_block(stack[i])) {
            if (Color(stack[i]) == WHITE) {
                change_color(&stack[i], GRAY);
                greys = pushHead(&stack[i], greys);
            }
        }
    }
    traverse_greys(greys);
    show(greys);
}

ml_list sweep(ml_list lst) {
    if (lst->val == NULL) {
        return lst;
    }
    while (lst != NULL && Color(*lst->val) == WHITE) {
        ml_list tofree = lst;
        lst = lst->next;
        free(tofree);
    }


    ml_list pred = lst;
    while (pred != NULL && pred->next != NULL) {
        while (pred->next != NULL && Color(*lst->next->val) == WHITE) {
            ml_list tofree = pred->next;
            pred->next = pred->next->next;
            free(tofree);
        }
        pred = pred->next;
    }
    return lst;
}
