#include <stdlib.h>
#include <stdio.h>

#include "domain_state.h"
#include "config.h"
#include "mlvalues.h"
#include "gc.h"
#include "lists.h"

caml_domain_state* Caml_state;

void caml_init_domain() {



    /*
     * mark and sweep
     */
    Caml_state->big_list=ml_empty_list();
    Caml_state->big_list_size=0;
    Caml_state->free_list=ml_empty_list();
    Caml_state->free_list_sz=0;
    Caml_state->page_list=ml_empty_list();
    /*
    * mark and sweep
    */

  Caml_state = malloc(sizeof(caml_domain_state));

  Caml_state->stack = malloc(Stack_size);

  Caml_state->space[0] = new_semispace(Semi_space_size);
  Caml_state->space[1] = new_semispace(Semi_space_size);
  Caml_state->current_semispace=0;

  Caml_state->remembered_set = new_ml_fifo();
}

void free_semispace(semi_space space){
    free(space->tas);
    free(space);
}

void free_domain(){
    free(Caml_state->stack);
    free_semispace(Caml_state->space[0]);
    free_semispace(Caml_state->space[1]);
    free_fifo(Caml_state->remembered_set);
    free(Caml_state);
}

semi_space new_semispace(size_t size){
    semi_space space = malloc(sizeof(struct _semi_space));
    space->tas = malloc(sizeof(mlvalue)*size);
    space->capcity = size;
    space->alloc_pointer = 0;
    return space;
}

mlvalue* allocate_in_semispace(size_t size){
    semi_space current_semispace = Caml_state->space[Caml_state->current_semispace];
    uint64_t start_alloc_pointer = current_semispace->alloc_pointer;
    if(start_alloc_pointer + size >= current_semispace->capcity){
        start_gc();
        current_semispace = Caml_state->space[Caml_state->current_semispace];
        start_alloc_pointer = current_semispace->alloc_pointer;
    }
    if(start_alloc_pointer + size >= current_semispace->capcity){
        perror("Memory overflow.\n");
        exit(EXIT_FAILURE);
    }
    current_semispace->alloc_pointer +=size;
    return current_semispace->tas + start_alloc_pointer;
}