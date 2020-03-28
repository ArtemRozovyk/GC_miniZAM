#ifndef _DOMAIN_STATE_H
#define _DOMAIN_STATE_H

#include "mlvalues.h"
#include "lists.h"



typedef struct pg {
    mlvalue * blocks;
    mlvalue current_block;
    ulong capacity;
    ulong size;

} * page;
typedef struct _caml_domain_state {
  /* Stack */
  mlvalue* stack;
  /*Big objects*/
  ml_list big_list;
  ulong big_list_size;
  ml_list page_list;
  ml_list free_list;
  ulong free_list_sz;
} caml_domain_state;

/* The global state */
extern caml_domain_state* Caml_state;

/* Initialisation function for |Caml_state| */
void caml_init_domain();

#endif
