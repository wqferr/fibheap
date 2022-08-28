#include "graph.h"
#include "generic.h"
#include "hashmap.h"

#include <stdlib.h>

struct Graph {
    HashMap *verts;
    hash_f hash;
    comp_f comp;
};

void _graph_freenode(gvalue key, gvalue gv_edges, void *arg) {
    HashMap *edges = (HashMap *) gv_edges.p;
    (void) key;
    (void) arg;
    hm_free(edges);
}

Graph *graph_new(hash_f hash, comp_f comp) {
    Graph *g = malloc(sizeof(*g));
    g->verts = hm_new(hash, comp);
    g->hash = hash;
    g->comp = comp;
    return g;
}

void graph_free(Graph *g) {
    hm_foreach(g->verts, _graph_freenode, NULL);
    hm_free(g->verts);
    free(g);
}

bool graph_newvert(Graph *g, gvalue key) {
    if (graph_hasvert(g, key))
        return false;

    gvalue edges;
    edges.p = hm_new(g->hash, g->comp);
    hm_put(g->verts, key, edges);
    return true;
}

bool graph_hasvert(const Graph *g, gvalue key) {
    return hm_has(g->verts, key);
}

bool graph_newedge(Graph *g, gvalue keystart, gvalue keyend, float weight) {
    gvalue gv_edges, gv_weight;
    if (!hm_get(g->verts, keystart, &gv_edges))
        return false;
    if (!graph_hasvert(g, keyend))
        return false;
    gv_weight.f = weight;
    return hm_put((HashMap *) gv_edges.p, keyend, gv_weight);
}

bool graph_edge(const Graph *g, gvalue keystart, gvalue keyend, float *outweight) {
    gvalue gv_edges, gv_weight;
    if (!hm_get(g->verts, keystart, &gv_edges))
        return false;
    if (!graph_hasvert(g, keyend))
        return false;
    if (!hm_get((HashMap *) gv_edges.p, keyend, &gv_weight))
        return false;
    *outweight = gv_weight.f;
    return true;
}

void _graph_writevert(gvalue key, gvalue edges, void *writeloc) {
    (void) edges;
    gvalue **loc = writeloc;
    **loc = key;
    (*loc)++;
}

bool graph_neighbors(const Graph *g, gvalue key, gvalue **outlist, size_t *outn) {
    gvalue gv_edges;
    if (!hm_get(g->verts, key, &gv_edges))
        return false;
    *outn = hm_size((HashMap *)gv_edges.p);
    gvalue *aux = realloc(*outlist, *outn * sizeof(*aux));
    *outlist = aux;
    hm_foreach((HashMap *)gv_edges.p, _graph_writevert, &aux);
    return true;
}

size_t graph_nverts(const Graph *g) {
    return hm_size(g->verts);
}