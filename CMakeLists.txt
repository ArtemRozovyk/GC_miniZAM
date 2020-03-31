cmake_minimum_required(VERSION 3.15)
project(GC_miniZAM C)

set(CMAKE_C_STANDARD 11)

include_directories(minizam-full/src)

add_executable(GC_miniZAM
        minizam-full/src/alloc.c
        minizam-full/src/alloc.h
        minizam-full/src/config.h
        minizam-full/src/domain.c
        minizam-full/src/domain_state.h
        minizam-full/src/instruct.h
        minizam-full/src/interp.c
        minizam-full/src/interp.h
        minizam-full/src/minizam.c
        minizam-full/src/mlvalues.c
        minizam-full/src/mlvalues.h
        minizam-full/src/parser.c
        minizam-full/src/parser.h
        minizam-full/src/primitives.c
        minizam-full/src/primitives.h
        minizam-full/src/gc.c
        minizam-full/src/gc.h
        minizam-full/src/gc.h
        minizam-full/src/fifo.c
        minizam-full/src/fifo.h minizam-full/src/lists.h minizam-full/src/lists.c)
