//
// Created by human on 24/03/2020.
//

#ifndef GC_MINIZAM_GC_H
#define GC_MINIZAM_GC_H


#include "lists.h"
ml_list mark(mlvalue * stack, unsigned int curr_stack_sz);

ml_list sweep(ml_list);
#endif //GC_MINIZAM_GC_H
