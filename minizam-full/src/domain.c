#include <stdlib.h>

#include "domain_state.h"
#include "config.h"
#include "mlvalues.h"


caml_domain_state* Caml_state;

void caml_init_domain() {

  Caml_state = malloc(sizeof(caml_domain_state));
  // size in mlvalues
  Caml_state->curr_page_space_size=0;
  Caml_state->old_page_space_size=10;
  Caml_state->big_list=ml_empty_list();
  Caml_state->big_list_size=0;
  Caml_state->free_list=ml_empty_list();
  Caml_state->free_list_sz=0;
  Caml_state->page_list=ml_empty_list();
  Caml_state->stack = malloc(Stack_size);
}
