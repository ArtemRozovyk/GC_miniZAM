//
// Created by hernouf on 26/03/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"

ml_fifo new_ml_fifo(){
    ml_fifo res = malloc(sizeof(struct mlf));
    res->start = NULL;
    res->end = NULL;
    return  res;
}

int is_empty_fifo(ml_fifo fifo){
    return fifo->start==NULL;
}

void push_fifo(ml_fifo fifo, void* place, mlvalue val){
    ml_fifo_field field = malloc(sizeof(struct mlff));
    context ctx;
    ctx.place = place;
    ctx.val = val;
    field->ctx=ctx;
    field->next=NULL;
    if(is_empty_fifo(fifo)){
        fifo->start = field;
        fifo->end = field;
    }
    else{
        fifo->end->next = field;
        fifo->end = field;
    }
}

context pop_fifo(ml_fifo fifo){
    context ctx;
    ctx.place = NULL;
    ctx.val = 0;
    if(is_empty_fifo(fifo)){
        return ctx;
    }
    else
    {
        ml_fifo_field field = fifo->start;
        context res = field->ctx;
        if(fifo->start==fifo->end){
            fifo->start=NULL;
            fifo->end=NULL;
        } else {
            fifo->start = field->next;
        }
        free(field);
        return res;
    }
}

void free_fifo(ml_fifo fifo){
    ml_fifo_field field = NULL;
    while (fifo->start != NULL){
        field = fifo->start;
        fifo->start = field->next;
        free(field);
    }
    free(fifo);
}