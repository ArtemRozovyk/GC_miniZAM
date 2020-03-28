//
// Created by human on 24/03/2020.
//

#ifndef GC_MINIZAM_LISTS_H
#define GC_MINIZAM_LISTS_H


#include <stdio.h>
#include <stdlib.h>
#include "mlvalues.h"

typedef struct mll *ml_list;

struct mll {
    mlvalue * val;
    ml_list next;
};

ml_list ml_empty_list();
ml_list pushHead(mlvalue * i,ml_list fl);
ml_list pushTail(mlvalue * i,ml_list fl);
void show (ml_list fl);
void release(ml_list fl);

void show_colors(ml_list fl);


#endif //GC_MINIZAM_LISTS_H
