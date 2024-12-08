#ifndef AVL_H
#define AVL_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct avl_tree AVL_TREE;

AVL_TREE *avl_tree_new(void);

bool avl_tree_insert(AVL_TREE *, int value);
bool avl_tree_remove(AVL_TREE *, int value);
bool avl_tree_search(AVL_TREE *, int value);

size_t avl_tree_len(AVL_TREE *);

void avl_tree_traverse(AVL_TREE *, void (*cb)(int value, void *ctx), void *ctx);

AVL_TREE *avl_tree_clone(AVL_TREE *);

AVL_TREE *avl_tree_union(AVL_TREE **, AVL_TREE **);
AVL_TREE *avl_tree_intersection(AVL_TREE **, AVL_TREE **);

void avl_tree_free(AVL_TREE **);

/* XXX: debugging stuff */
void avl_tree_print(AVL_TREE *, void *);

#endif
