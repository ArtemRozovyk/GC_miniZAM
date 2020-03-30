#ifndef _DOMAIN_STATE_H
#define _DOMAIN_STATE_H

#include "mlvalues.h"
#include "fifo.h"

typedef struct _semi_space{
    mlvalue* tas;
    uint64_t capcity;
    uint64_t alloc_pointer;
} *semi_space;

typedef struct _caml_domain_state {
  /* Stack */
  mlvalue* stack;
  /* Tas */
  semi_space space[2];
  int current_semispace;
  ml_fifo remembered_set;
} caml_domain_state;

/* The global state */
extern caml_domain_state* Caml_state;

/* Initialisation function for |Caml_state| */
void caml_init_domain();
void free_semispace(semi_space);
semi_space new_semispace(size_t);
void free_domain();
mlvalue* allocate_in_semispace(size_t);

#endif
