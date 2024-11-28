#ifndef AVL_H
#define AVL_H

typedef struct avl_tree AVL_TREE;

AVL_TREE *avl_tree_new(void);

void avl_tree_free(AVL_TREE **);

#endif
