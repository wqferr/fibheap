#ifndef GRAPH_H
#define GRAPH_H

#include "generic.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct Graph Graph;

Graph *graph_new(hash_f, comp_f);
void graph_free(Graph *);
bool graph_newvert(Graph *, gvalue key);
bool graph_hasvert(const Graph *, gvalue key);
bool graph_newedge(Graph *, gvalue keystart, gvalue keyend, float weight);
bool graph_edge(const Graph *, gvalue keystart, gvalue keyend, float *outweight);

// works similarly to getline, variable should contain NULL if no buffer is allocated
bool graph_neighbors(const Graph *, gvalue key, gvalue **outlist, size_t *outn);

size_t graph_nverts(const Graph *);

#endif
