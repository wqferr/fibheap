#ifndef HASHMAP_H
#define HASHMAP_H

#include "generic.h"

#include <stddef.h>
#include <stdbool.h>

#define HM_DEFAULT_MAX_LOAD_FACTOR (0.75)
#define HM_MIN_SIZE (32)

typedef struct HashMap HashMap;

HashMap *hm_new(hash_f, comp_f);
void hm_free(HashMap *);

// check if hashmap has a given key
bool hm_has(const HashMap *, gvalue key);

// insert or replace a key's associated value; return true if key existed before
bool hm_put(HashMap *, gvalue key, gvalue val);

// retrieve value associated with given key, if it exists, and store it in outval;
// return true if key existed
bool hm_get(const HashMap *, gvalue key, gvalue *outval);

// remove value associated with given key, if it exists, and store it in outval; return true
// if key existed
bool hm_del(HashMap *, gvalue key, gvalue *outval);

// insert or replace value associated with given key; if it existed before, store the old
// value in val and return true
bool hm_replace(HashMap *, gvalue key, gvalue *val);

// number of entries in the hashmap
size_t hm_size(const HashMap *);

// apply a function to every key-value pair in the hashmap
void hm_foreach(HashMap *, void (*)(gvalue, gvalue, void *), void *arg);
void hm_setmaxload(HashMap *, float);

#endif