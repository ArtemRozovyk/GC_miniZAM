#ifndef _CONFIG_H
#define _CONFIG_H

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

/* The size of the stack. */
/* Note that it doesn't hold 8MB mlvalues, but rather 8MB of bytes. */
/* No boundary-checks are done: stack overflow will silently corrupt
   the heap; probably causing something to go wrong somewhere. */
#define Stack_size (8 * MB)
#define Semi_space_size (32 * KB)
#define Semi_space_min_limit (1 * KB)
#define Page_size (64 * KB)

#endif
