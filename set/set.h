#ifndef SET_H
#define SET_H

#include <stdbool.h>

typedef struct set SET;

enum set_type {
    SET_AVL,
    SET_RB,
    SET_MAX
};

SET *set_new(enum set_type type);

bool set_contains(SET *, int value);

bool set_insert(SET *, int value);
bool set_remove(SET *, int value);

SET *set_union(SET *, SET *);
SET *set_intersection(SET *, SET *);

void set_print(SET *);

void set_free(SET **);

#endif
