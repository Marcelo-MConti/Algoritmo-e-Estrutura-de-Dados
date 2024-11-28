#ifndef RB_H
#define RB_H

typedef struct rb_tree RB_TREE;

RB_TREE *rb_tree_new(void);

void rb_tree_free(RB_TREE **);

#endif

