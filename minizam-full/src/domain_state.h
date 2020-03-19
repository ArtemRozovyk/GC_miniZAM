#ifndef _DOMAIN_STATE_H
#define _DOMAIN_STATE_H

#include "mlvalues.h"

typedef struct _semi_space{
    mlvalue* tas;
    int capcity;
    int alloc_pointer;
} *semi_space;

typedef struct _caml_domain_state {
  /* Stack */
  mlvalue* stack;
  /* Tas */
  semi_space space[2];
  int current_semispace;
} caml_domain_state;

/* The global state */
extern caml_domain_state* Caml_state;

/* Initialisation function for |Caml_state| */
void caml_init_domain();
semi_space new_semispace(int);
void free_semispace(semi_space);
mlvalue* allocate_in_semispace(int);

#endif
