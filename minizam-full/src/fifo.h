//
// Created by hernouf on 26/03/2020.
//

#ifndef GC_MINIZAM_FIFO_H
#define GC_MINIZAM_FIFO_H

#include "mlvalues.h"

typedef struct _context {
    void* place;
    mlvalue val;
} context;

typedef struct mlff {
    context ctx;
    struct mlff* next;
}* ml_fifo_field;

typedef struct mlf {
    ml_fifo_field start;
    ml_fifo_field end;
}* ml_fifo;

ml_fifo new_ml_fifo();
int is_empty_fifo(ml_fifo);
void push_fifo(ml_fifo, void*, mlvalue);
context pop_fifo(ml_fifo);
void free_fifo(ml_fifo);

#endif //GC_MINIZAM_FIFO_H
