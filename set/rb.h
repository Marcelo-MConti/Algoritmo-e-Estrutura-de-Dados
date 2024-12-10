#ifndef RB_H
#define RB_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct rb_tree RB_TREE;

RB_TREE *rb_tree_new(void);

bool rb_tree_insert(RB_TREE *, int value);
bool rb_tree_remove(RB_TREE *, int value);
bool rb_tree_search(RB_TREE *, int value);

size_t rb_tree_len(RB_TREE *);

void rb_tree_traverse(RB_TREE *, void (*cb)(int value, void *ctx), void *ctx);

RB_TREE *rb_tree_clone(RB_TREE *);

void rb_tree_free(RB_TREE **);

#endif

