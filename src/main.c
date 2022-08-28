#define _POSIX_C_SOURCE 202208L
#define _ISOC99_SOURCE

#include "hashmap.h"
#include "pqueue.h"
#include "graph.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <stdio.h>

hash_type hashstr(gvalue v) {
    uint64_t h = 0;
    for (const char *s = v.p; *s != '\0'; s++)
        h = 31 * h + *s;
    return h;
}

bool compstr(gvalue stra, gvalue strb) {
    return strcmp(stra.p, strb.p) == 0;
}

void printentry(gvalue key, gvalue val, void *arg) {
    (void) arg;
    printf("%s: %ld\n", (char *) key.p, val.i);
}

void pushpq(PQueue *pq, const char *key, prio_type prio) {
    gvalue gv_key;
    gv_key.p = strdup(key);
    bool shouldfree = pq_has(pq, gv_key);
    pq_push(pq, gv_key, prio);
    if (shouldfree)
        free(gv_key.p);
}

void freekey(gvalue key, gvalue val, void *arg) {
    (void) val;
    (void) arg;
    free(key.p);
}

void printpqentry(gvalue key, prio_type prio, void *arg) {
    (void) arg;
    printf("%s\t%f\n", (char*)key.p, prio);
}

void freepqkey(gvalue key, prio_type prio, void *arg) {
    (void) prio;
    (void) arg;
    free(key.p);
}

hash_type hashint(gvalue gv) {
    return (hash_type) gv.i;
}

bool compint(gvalue a, gvalue b) {
    return a.i == b.i;
}

int main_hm(void) {
    HashMap *hm = hm_new(hashstr, compstr);
    char *buffer = NULL;
    char *valstr, *convfail;
    gvalue key, val;
    size_t bufsize;
    do {
        getline(&buffer, &bufsize, stdin);
        printf("\n");
        key.p = strtok(buffer, " \n");
        if (key.p == NULL)
            break;
        valstr = strtok(NULL, " \n");
        if (valstr == NULL) {
            if (hm_get(hm, key, &val)) {
                printf("%s: %ld\n", (char *) key.p, val.i);
            } else {
                printf("%s does not exist\n", (char *) key.p);
            }
        } else {
            val.i = strtol(valstr, &convfail, 10);
            if (convfail == valstr) {
                // failed number conversion
                continue;
            }
            key.p = strdup(key.p);
            if (hm_put(hm, key, val))
                free(key.p); // duplicate key, get rid of this one
            hm_foreach(hm, printentry, NULL);
        }
        printf("\n\n");
    } while (1);
    free(buffer);
    hm_foreach(hm, freekey, NULL);
    hm_free(hm);
    return 0;
}

int main_pq(void) {
    PQueue *pq = pq_new(hashstr, compstr);
    gvalue key;
    float prio;
    size_t size;

    pushpq(pq, "A", 1);
    pushpq(pq, "E", 5);
    pushpq(pq, "C", 3);
    pushpq(pq, "B", 2);
    pushpq(pq, "D", 4);
    while (pq_size(pq) > 0) {
        size = pq_size(pq);
        pq_pop(pq, &key, &prio);
        printf("%zu->%zu\t%s: %f\n", size, pq_size(pq), (const char *)key.p, prio);
        free(key.p);
    }

    printf("\n");
    pushpq(pq, "D", 4);
    pushpq(pq, "C", 3);
    pushpq(pq, "A", 1);
    pushpq(pq, "B", 2);
    pq_pop(pq, &key, NULL);
    printf("%s\n", (char *)key.p);
    free(key.p);

    pushpq(pq, "E", 5);
    pq_pop(pq, &key, NULL);
    printf("%s\n", (char *)key.p);
    free(key.p);

    pushpq(pq, "E", 2);
    pq_pop(pq, &key, NULL);
    printf("%s\n", (char *)key.p);
    free(key.p);

    pq_foreach(pq, freepqkey, NULL);
    pq_free(pq);
    return 0;
}

int main_dijkstra_adjmat(void) {
    #define NVERT (5)
    int ADJMAT[NVERT][NVERT] = {
        {0, 3, 2, 0, 0},
        {3, 0, 5, 3, 0},
        {2, 5, 0, 0, 20},
        {0, 3, 0, 0, 4},
        {0, 0, 20, 4, 0}
    };
    const int start = 0;
    const int end = 4;
    gvalue key;
    gvalue cur;
    HashMap *prev = hm_new(hashint, compint);
    PQueue *open = pq_new(hashint, compint);

    key.i = start;
    pq_push(open, key, 0);
    while (pq_size(open) > 0) {
        gvalue neighbor;
        float curdist;
        pq_pop(open, &cur, &curdist);
        if (cur.i == end)
            break;
        for (neighbor.i = 0; neighbor.i < NVERT; neighbor.i++) {
            if (ADJMAT[cur.i][neighbor.i] != 0) { // is a neighbor
                float altdist = curdist + ADJMAT[cur.i][neighbor.i];
                if (pq_push(open, neighbor, altdist))
                    hm_put(prev, neighbor, cur);
            }
        }
    }

    if (cur.i == end) {
        printf("%ld", cur.i);
        while (cur.i != start) {
            hm_get(prev, cur, &cur);
            printf(" <- %ld", cur.i);
        }
        puts("");
    } else {
        puts("no solution");
    }

    pq_free(open);
    hm_free(prev);
    return 0;
}

void doubleedge(Graph *g, gvalue keya, gvalue keyb, float weight) {
    graph_newedge(g, keya, keyb, weight);
    graph_newedge(g, keyb, keya, weight);
}

Graph *biggraph(void) {
    // from https://stackoverflow.com/questions/10254542/dijkstras-algorithm-does-not-generate-shortest-path
    Graph *g = graph_new(hashstr, compstr);
    graph_newvert(g, "a");
    graph_newvert(g, "b");
    graph_newvert(g, "c");
    graph_newvert(g, "d");
    graph_newvert(g, "e");
    graph_newvert(g, "f");
    graph_newvert(g, "g");
    graph_newvert(g, "h");
    graph_newvert(g, "i");
    graph_newvert(g, "j");
    graph_newvert(g, "k");
    graph_newvert(g, "l");
    graph_newvert(g, "m");
    graph_newvert(g, "n");
    graph_newvert(g, "o");
    graph_newvert(g, "p");
    graph_newvert(g, "q");
    graph_newvert(g, "r");
    graph_newvert(g, "s");
    graph_newvert(g, "t");
    graph_newvert(g, "z");

    doubleedge(g, "a", "b", 2);
    doubleedge(g, "a", "c", 4);
    doubleedge(g, "a", "d", 1);

    doubleedge(g, "b", "c", 3);
    doubleedge(g, "b", "e", 1);

    doubleedge(g, "c", "e", 2);
    doubleedge(g, "c", "f", 2);

    doubleedge(g, "d", "f", 5);
    doubleedge(g, "d", "g", 4);

    doubleedge(g, "e", "h", 3);

    doubleedge(g, "f", "h", 3);
    doubleedge(g, "f", "i", 2);
    doubleedge(g, "f", "j", 4);
    doubleedge(g, "f", "g", 3);

    doubleedge(g, "g", "k", 2);

    doubleedge(g, "h", "o", 8);
    doubleedge(g, "h", "l", 1);

    doubleedge(g, "i", "l", 3);
    doubleedge(g, "i", "m", 2);
    doubleedge(g, "i", "j", 3);

    doubleedge(g, "j", "m", 6);
    doubleedge(g, "j", "n", 3);
    doubleedge(g, "j", "k", 6);

    doubleedge(g, "k", "n", 4);
    doubleedge(g, "k", "r", 2);
    
    doubleedge(g, "l", "m", 3);
    doubleedge(g, "l", "o", 6);

    doubleedge(g, "m", "o", 4);
    doubleedge(g, "m", "p", 2);
    doubleedge(g, "m", "n", 5);

    doubleedge(g, "n", "q", 2);
    doubleedge(g, "n", "r", 1);

    doubleedge(g, "o", "p", 2);
    doubleedge(g, "o", "s", 6);

    doubleedge(g, "p", "q", 1);
    doubleedge(g, "p", "s", 2);
    doubleedge(g, "p", "t", 1);

    doubleedge(g, "q", "r", 8);
    doubleedge(g, "q", "t", 3);

    doubleedge(g, "r", "t", 5);

    doubleedge(g, "s", "z", 2);

    doubleedge(g, "t", "z", 8);

    return g;
}

int main_dijkstra(void) {
    Graph *g = biggraph();
    HashMap *prev = hm_new(hashstr, compstr);
    PQueue *open = pq_new(hashstr, compstr);
    HashMap *closed = hm_new(hashstr, compstr);
    gvalue current;
    gvalue start, end;
    gvalue *neighbors = NULL;
    size_t neighsize = 0;
    float edgeweight;

    start.p = "a";
    end.p = "z";
    pq_push(open, start, 0);
    current = start;
    while (pq_size(open) > 0) {
        float curdist;
        pq_foreach(open, printpqentry, NULL);
        printf("-----\n");
        pq_pop(open, &current, &curdist);
        if (compstr(current, end))
            break;
        hm_put(closed, current, 0);
        graph_neighbors(g, current, &neighbors, &neighsize);
        for (size_t i = 0; i < neighsize; i++) {
            graph_edge(g, current, neighbors[i], &edgeweight);
            float altdist = curdist + edgeweight;
            if (!hm_has(closed, neighbors[i]) && pq_push(open, neighbors[i], altdist))
                hm_put(prev, neighbors[i], current);
        }
    }
    free(neighbors);
    if (compstr(current, end)) {
        printf("%s", (char *)current.p);
        while (!compstr(current, start)) {
            hm_get(prev, current, &current);
            printf(" <- %s", (char *)current.p);
        }
        puts("");
    } else {
        puts("no solution");
    }

    pq_free(open);
    hm_free(prev);
    hm_free(closed);
    graph_free(g);
    return 0;
}

int main(void) {
    return main_dijkstra();
}