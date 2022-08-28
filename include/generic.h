#ifndef GENERIC_H
#define GENERIC_H

#include <stdbool.h>
#include <stdint.h>

typedef union {
    intptr_t i;
    double f; // this is assuming sizeof(double) == sizeof(void*) == 8
    void *p;
} gvalue
#ifdef __GNUC__
__attribute__ ((__transparent_union__))
#endif
;

typedef uint64_t hash_type;

typedef hash_type (*hash_f)(gvalue);
typedef bool (*comp_f)(gvalue, gvalue);

#endif