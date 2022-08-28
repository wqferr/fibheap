#include "pqueue.h"
#include "hashmap.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

static const int MERGE_VEC_SLOTS = 50;

typedef struct PQNode {
    gvalue key;
    prio_type prio;
    bool cutchild;

    struct PQNode *parent;
    struct PQNode *lsibling;
    struct PQNode *rsibling;
    struct PQNode *childlist;
} PQNode;

struct PQueue {
    PQNode *minroot;
    PQNode *rootlist;
    HashMap *nodes;
};

#include <stdio.h>

// all lists have a sentinel "head" node which loops around beginning and end.
// this is the node given here as list, and as node->childlist. sentinel nodes
// have their childlist == NULL
void _pq_appendnode(PQNode *list, PQNode *newnode) {
    newnode->rsibling = list;
    newnode->lsibling = list->lsibling;
    list->lsibling->rsibling = newnode;
    list->lsibling = newnode;
}

PQNode *_pq_newsentinel(PQNode *parent) {
    PQNode *node = malloc(sizeof(*node));
    node->parent = parent;
    node->lsibling = node->rsibling = node;
    node->childlist = NULL;
    return node;
}

PQNode *_pq_newnode(PQNode *parent, PQNode *siblinglist, gvalue key, prio_type prio) {
    PQNode *node = malloc(sizeof(*node));
    node->key = key;
    node->prio = prio;

    node->parent = parent;
    node->cutchild = false;
    node->childlist = _pq_newsentinel(node);

    _pq_appendnode(siblinglist, node);
    return node;
}

void _pq_remove(PQNode *node) {
    node->lsibling->rsibling = node->rsibling;
    node->rsibling->lsibling = node->lsibling;
    node->lsibling = node->rsibling = NULL;
    node->parent = NULL;
}

void _pq_sever(PQNode *node, PQNode *rootlist) {
    if (node->parent != NULL) {
        if (node->parent->cutchild) {
            _pq_sever(node->parent, rootlist);
        } else {
            node->parent->cutchild = true;
        }
    }
    _pq_remove(node);
    _pq_appendnode(rootlist, node);
}

void _pq_freelist(PQNode *list) {
    PQNode *next, *cur = list->rsibling;
    while (cur != list) {
        _pq_freelist(cur->childlist);
        next = cur->rsibling;
        free(cur);
        cur = next;
    }
    free(cur);
}

void _pq_foreach(const PQNode *list, void (*func)(gvalue, prio_type, void *), void *arg) {
    const PQNode *cur = list->rsibling;
    while (cur != list) {
        _pq_foreach(cur->childlist, func, arg);
        func(cur->key, cur->prio, arg);
        cur = cur->rsibling;
    }
}

size_t _pq_degree(const PQNode *node) {
    size_t d = 0;
    PQNode *cur = node->childlist->rsibling;
    while (cur != node->childlist) {
        d++;
        cur = cur->rsibling;
    }
    return d;
}

PQNode *_pq_mergesingle(PQNode *a, PQNode *b) {
    if (b->prio < a->prio)
        return _pq_mergesingle(b, a);
    a->cutchild = b->cutchild = false;
    _pq_remove(b);
    _pq_appendnode(a->childlist, b);
    b->parent = a;
    return a;
}

void _pq_merge(PQNode *mergevec[], size_t idx, PQNode *new) {
    assert(idx < MERGE_VEC_SLOTS);
    PQNode *merged = _pq_mergesingle(new, mergevec[idx]);
    mergevec[idx] = NULL;
    if (mergevec[idx+1] == NULL) {
        mergevec[idx+1] = merged;
    } else {
        _pq_merge(mergevec, idx+1, merged);
    }
}

// returns min node
PQNode *_pq_mergelist(PQNode *rootlist) {
    PQNode *mergevec[MERGE_VEC_SLOTS];
    PQNode *minnode = rootlist->rsibling;
    for (size_t i = 0; i < MERGE_VEC_SLOTS; i++)
        mergevec[i] = NULL;
    PQNode *cur = rootlist->rsibling;
    PQNode *next;
    if (cur == rootlist) return NULL;

    while (cur != rootlist) {
        next = cur->rsibling;
        if (cur->prio < minnode->prio)
            minnode = cur;
        size_t degree = _pq_degree(cur);
        if (mergevec[degree] == NULL) {
            mergevec[degree] = cur;
        } else {
            _pq_merge(mergevec, degree, cur);
        }
        cur = next;
    }

    return minnode;
}

PQueue *pq_new(hash_f hash, comp_f comp) {
    PQueue *p = malloc(sizeof(*p));
    p->nodes = hm_new(hash, comp);
    p->rootlist = _pq_newsentinel(NULL);
    p->minroot = NULL;
    return p;
}

void pq_free(PQueue *p) {
    _pq_freelist(p->rootlist);
    hm_free(p->nodes);
    free(p);
}

bool pq_push(PQueue *p, gvalue key, prio_type prio) {
    gvalue gv_node;
    if (hm_get(p->nodes, key, &gv_node)) {
        PQNode *existing = gv_node.p;
        if (prio >= existing->prio)
            return false;

        existing->prio = prio;
        if (existing->parent != NULL && existing->parent->prio >= prio)
            _pq_sever(existing, p->rootlist);
    } else {
        gv_node.p = _pq_newnode(NULL, p->rootlist, key, prio);
        hm_put(p->nodes, key, gv_node);
    }
    if (p->minroot == NULL || prio < p->minroot->prio)
        p->minroot = gv_node.p;
    return true;
}

bool pq_pop(PQueue *p, gvalue *outkey, prio_type *outprio) {
    if (!pq_peek(p, outkey, outprio))
        return false;
    hm_del(p->nodes, p->minroot->key, NULL);
    PQNode *next, *cur = p->minroot->childlist->rsibling;
    while (cur != p->minroot->childlist) {
        next = cur->rsibling;
        _pq_remove(cur);
        _pq_appendnode(p->rootlist, cur);
        cur = next;
    }
    _pq_remove(p->minroot);
    free(cur);
    free(p->minroot);
    p->minroot = _pq_mergelist(p->rootlist);

    return true;
}

bool pq_peek(const PQueue *p, gvalue *outkey, prio_type *outprio) {
    PQNode *m = p->minroot;
    if (m == NULL)
        return false;
    if (outkey != NULL)
        *outkey = m->key;
    if (outprio != NULL)
        *outprio = m->prio;
    return true;
}

bool pq_find(const PQueue *p, gvalue key, prio_type *outprio) {
    gvalue gv_node;
    if (!hm_get(p->nodes, key, &gv_node))
        return false;
    PQNode *node = (PQNode *) gv_node.p;
    *outprio = node->prio;
    return true;
}

bool pq_has(const PQueue *p, gvalue key) {
    return hm_has(p->nodes, key);
}

size_t pq_size(const PQueue *p) {
    return hm_size(p->nodes);
}

void pq_foreach(PQueue *p, void (*func)(gvalue, prio_type, void *), void *arg) {
    _pq_foreach(p->rootlist, func, arg);
}