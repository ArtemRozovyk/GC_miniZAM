#ifndef _DOMAIN_STATE_H
#define _DOMAIN_STATE_H

#include "mlvalues.h"
#include "lists.h"


typedef struct _caml_domain_state {
  /* Stack */
  mlvalue* stack;
  /*Big objects*/
  ml_list big_list;
  int big_list_size;
  ml_list page_list;
  ml_list free_list;
  long nballoc;
  long nbfree;
} caml_domain_state;

/* The global state */
extern caml_domain_state* Caml_state;

/* Initialisation function for |Caml_state| */
void caml_init_domain();

#endif
