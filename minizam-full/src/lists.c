//
// Created by human on 24/03/2020.
//



#include "lists.h"

ml_list ml_empty_list() {
    ml_list f = malloc(sizeof(struct mll));
    f->val = NULL;
    f->next = NULL;
    return f;
}

ml_list pushHead(mlvalue *i, ml_list fl) {
    //on suppose fl non NULL;
    if (fl->val == NULL) {
        fl->val = i;
        return fl;
    } else {
        ml_list nfl = ml_empty_list();
        nfl->val = i;
        nfl->next = fl;
        return nfl;
    }
}
ml_list pushTail(mlvalue * i,ml_list fl){
    if (fl->val == NULL) {
        fl->val = i;
        return fl;
    }else{
        ml_list ll=fl;
        while (ll->next){
            ll=ll->next;
        }
        ll->next=ml_empty_list();
        ll->next->val=i;
    }
    return fl;
}


void show(ml_list fl) {
    ml_list tmp = fl;
    while (tmp && tmp->val) {
        //printf("%p - ", tmp->val);
        tmp = tmp->next;
    }
}

void show_colors(ml_list fl) {
    ml_list tmp = fl;
    while (tmp && tmp->val) {
        //printf("%p - ", tmp->val);
        switch (Color(tmp->val)) {
            case WHITE:
                printf(" WHITE %p ", tmp->val);
                break;
            case GRAY:
                printf(" GRAY %p ", tmp->val);
                break;
            case BLACK:
                printf(" BLACK %p ", tmp->val);
                break;
        }
        tmp = tmp->next;
    }
}

void release(ml_list fl) {
    if (!fl)return;
    if (fl->next == NULL) {
        return;
    } else {
        release(fl->next);
    }
    free(fl->next);
}

