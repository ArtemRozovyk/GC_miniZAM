#ifndef _INTERP_H
#define _INTERP_H

#include "mlvalues.h"
extern mlvalue * stack ;
extern mlvalue accu;
extern mlvalue env;
extern unsigned int sp;
mlvalue caml_interprete(code_t* prog);

#endif /* _INTERP_H */
