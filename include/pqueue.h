#ifndef PQUEUE_H
#define PQUEUE_H

#include <stddef.h>
#include <stdbool.h>

#include "generic.h"

typedef struct PQueue PQueue;

typedef float prio_type;

PQueue *pq_new(hash_f, comp_f);
void pq_free(PQueue *);

bool pq_push(PQueue *, gvalue key, prio_type prio); // also updates prio if it's lower than current
bool pq_peek(const PQueue *, gvalue *outkey, prio_type *outprio);
bool pq_pop(PQueue *, gvalue *outkey, prio_type *outprio);
bool pq_find(const PQueue *, gvalue key, prio_type *outprio);
bool pq_has(const PQueue *, gvalue key);

size_t pq_size(const PQueue *);
void pq_foreach(PQueue *, void (*)(gvalue, prio_type, void *), void *);

#endif