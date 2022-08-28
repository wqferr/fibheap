#include "generic.h"
#include "hashmap.h"

#include <stdlib.h>

typedef struct HashNode {
    struct HashNode *next;
    struct HashNode *prev;

    gvalue key;
    gvalue value;
} HashNode;

struct HashMap {
    HashNode **buckets;
    size_t nbuckets; // size of buckets array
    size_t size; // number of existing entries
    size_t nfilled; // number of filled buckets

    float maxload;

    hash_f hash;
    comp_f comp;
};

HashNode *_hm_newheadnode(void) {
    HashNode *n = malloc(sizeof(*n));
    n->next = n->prev = n;
    return n;
}

HashNode **_hm_newbucketlist(size_t size) {
    HashNode **list = malloc(size * sizeof(*list));
    for (size_t i = 0; i < size; i++)
        list[i] = _hm_newheadnode();
    return list;
}

void _hm_delbucketlist(HashNode **buckets, size_t nbuckets) {
    for (size_t i = 0; i < nbuckets; i++) {
        HashNode *head = buckets[i];
        HashNode *next;
        HashNode *cur = head->next;
        while (cur != head) {
            next = cur->next;
            free(cur);
            cur = next;
        }
        free(head);
    }
    free(buckets);
}

HashMap *hm_new(hash_f hash, comp_f comp) {
    HashMap *hm = malloc(sizeof(*hm));

    hm->buckets = _hm_newbucketlist(HM_MIN_SIZE);
    hm->nbuckets = HM_MIN_SIZE;
    hm->size = 0;
    hm->nfilled = 0;

    hm->maxload = HM_DEFAULT_MAX_LOAD_FACTOR;

    hm->hash = hash;
    hm->comp = comp;

    return hm;
}

void hm_free(HashMap *hm) {
    _hm_delbucketlist(hm->buckets, hm->nbuckets);
    free(hm);
}

bool _hm_find(const HashMap *hm, gvalue key, HashNode **outnode) {
    size_t idx = hm->hash(key) % hm->nbuckets;
    HashNode *headnode = hm->buckets[idx];
    HashNode *cur = headnode->next;

    while (cur != headnode) {
        if (hm->comp(cur->key, key)) {
            *outnode = cur;
            return true;
        }
        cur = cur->next;
    }
    *outnode = headnode;
    return false;
}

void _hm_rehash(HashMap *hm, size_t newsize) {
    HashNode **oldbuckets = hm->buckets;
    size_t oldsize = hm->nbuckets;
    if (newsize < HM_MIN_SIZE)
        newsize = HM_MIN_SIZE;

    hm->buckets = _hm_newbucketlist(newsize);
    hm->nfilled = 0;
    hm->size = 0;
    hm->nbuckets = newsize;
    
    for (size_t i = 0; i < oldsize; i++) {
        HashNode *head = oldbuckets[i];
        for (HashNode *cur = head->next; cur != head; cur = cur->next) {
            hm_put(hm, cur->key, cur->value);
        }
    }
    _hm_delbucketlist(oldbuckets, oldsize);
}

bool hm_has(const HashMap *hm, gvalue key) {
    HashNode *node;
    return _hm_find(hm, key, &node);
}

bool hm_get(const HashMap *hm, gvalue key, gvalue *outvalue) {
    HashNode *n;
    if (!_hm_find(hm, key, &n)) return false;

    *outvalue = n->value;
    return true;
}

bool hm_del(HashMap *hm, gvalue key, gvalue *outvalue) {
    HashNode *n, *next;
    if (!_hm_find(hm, key, &n)) return false;

    if (outvalue != NULL)
        *outvalue = n->value;

    next = n->next;
    n->prev->next = n->next;
    n->next->prev = n->prev;
    free(n);
    hm->size--;

    if (next->next == next) {
        // bucket is now empty
        hm->nfilled--;
        
        if (hm->nbuckets > HM_MIN_SIZE && hm->nfilled * 4 <= hm->maxload * hm->nbuckets) {
            // below alpha/4 load factor
            _hm_rehash(hm, hm->nbuckets / 2);
        }
    }

    return true;
}

bool hm_replace(HashMap *hm, gvalue key, gvalue *value) {
    HashNode *n, *newnode;
    gvalue aux;
    if (_hm_find(hm, key, &n)) {
        // already exists
        aux = n->value;
        n->value = *value;
        *value = aux;
        return true;
    } else {
        // n contains head node of correct bucket
        if (n->next == n) {
            // bucket empty
            hm->nfilled++;
        }

        newnode = malloc(sizeof(*newnode));
        newnode->key = key;
        newnode->value = *value;
        newnode->next = n;
        newnode->prev = n->prev;
        newnode->prev->next = newnode;
        newnode->next->prev = newnode;
        hm->size++;

        if (hm->nfilled >= hm->maxload * hm->nbuckets)
            _hm_rehash(hm, 2 * hm->nbuckets);

        return false;
    }
}

bool hm_put(HashMap *hm, gvalue key, gvalue value) {
    return hm_replace(hm, key, &value);
}



size_t hm_size(const HashMap *hm) {
    return hm->size;
}

void hm_foreach(HashMap *hm, void (*consumer)(gvalue, gvalue, void *), void *arg) {
    for (size_t i = 0; i < hm->nbuckets; i++) {
        HashNode *headnode = hm->buckets[i];
        for (HashNode *node = headnode->next; node != headnode; node = node->next) {
            consumer(node->key, node->value, arg);
        }
    }
}