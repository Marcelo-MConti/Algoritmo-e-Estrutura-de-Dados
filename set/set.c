#include <stdlib.h>

#include "rb.h"
#include "avl.h"

#include "set.h"

struct set {
    enum set_type type;
    union {
        AVL_TREE *avl;
        RB_TREE *rb;
    } impl;
};

SET *set_new(enum set_type type)
{
    SET *set = malloc(sizeof *set);

    if (!set)
        return NULL;

    set->type = type;

    switch (set->type) {
        case SET_AVL:
            set->impl.avl = avl_tree_new();
            break;
        case SET_RB:
            set->impl.rb = rb_tree_new();
            break;
    }

    return set;
}
