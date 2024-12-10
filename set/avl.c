#include <stdio.h>

#include "avl.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define UPDATE_HEIGHT(x)                                                                 \
    do {                                                                                 \
        (x)->height = MAX(avl_height((x)->left), avl_height((x)->right)) + 1;            \
    } while (0)

typedef struct avl_node {
    int value;
    int height;

    struct avl_node *left;
    struct avl_node *right;
} AVL_NODE;

typedef struct avl_split {
    AVL_NODE *left;
    AVL_NODE *right;

    bool present;
} AVL_SPLIT;

typedef struct avl_split_last {
    AVL_NODE *root;
    int value;
} AVL_SPLIT_LAST;

struct avl_tree {
    AVL_NODE *root;
    size_t len;
};

void avl_free_impl(AVL_NODE *);

AVL_TREE *avl_tree_new(void)
{
    AVL_TREE *tree = malloc(sizeof *tree);

    tree->root = NULL;
    tree->len = 0;

    return tree;
}

static AVL_NODE *avl_node_new(int value)
{
    AVL_NODE *node = malloc(sizeof *node);

    node->value = value;

    node->left = NULL;
    node->right = NULL;

    node->height = 0;

    return node;
}

static int avl_height(AVL_NODE *node)
{
    if (!node)
        return -1;

    return node->height;
}

static AVL_NODE *avl_rotate_left(AVL_NODE *node)
{
    AVL_NODE *right = node->right;

    node->right = right->left;
    right->left = node;

    UPDATE_HEIGHT(node);
    UPDATE_HEIGHT(right);

    return right;
}

static AVL_NODE *avl_rotate_right(AVL_NODE *node)
{
    AVL_NODE *left = node->left;

    node->left = left->right;
    left->right = node;

    UPDATE_HEIGHT(node);
    UPDATE_HEIGHT(left);

    return left;
}

static AVL_NODE *avl_rotate_left_right(AVL_NODE *node)
{
    node->left = avl_rotate_left(node->left);
    return avl_rotate_right(node);
}

static AVL_NODE *avl_rotate_right_left(AVL_NODE *node)
{
    node->right = avl_rotate_right(node->right);
    return avl_rotate_left(node);
}

static AVL_NODE *avl_join_right(AVL_NODE *left, int k, AVL_NODE *right)
{
    if (avl_height(left->right) <= avl_height(right) + 1) {
        AVL_NODE *new = avl_node_new(k);

        new->left = left->right;
        new->right = right;

        UPDATE_HEIGHT(new);

        left->right = new;

        if (new->height > avl_height(left->left) + 1)
            left = avl_rotate_right_left(left);
        else
            UPDATE_HEIGHT(left);

        return left;
    }

    AVL_NODE *new = avl_join_right(left->right, k, right);

    left->right = new;

    if (new->height > avl_height(left->left) + 1)
        left = avl_rotate_left(left);
    else
        UPDATE_HEIGHT(left);

    return left;
}

static AVL_NODE *avl_join_left(AVL_NODE *left, int k, AVL_NODE *right)
{
    if (avl_height(right->left) <= avl_height(left) + 1) {
        AVL_NODE *new = avl_node_new(k);

        new->left = left;
        new->right = right->left;

        UPDATE_HEIGHT(new);

        right->left = new;

        if (new->height > avl_height(right->right) + 1)
            right = avl_rotate_left_right(right);
        else
            UPDATE_HEIGHT(right);

        return right;
    }

    AVL_NODE *new = avl_join_left(left, k, right->left);

    right->left = new;

    if (new->height > avl_height(right->right) + 1)
        right = avl_rotate_right(right);
    else
        UPDATE_HEIGHT(right);

    return right;
}

static AVL_NODE *avl_join(AVL_NODE *left, int k, AVL_NODE *right)
{
    if (avl_height(left) > avl_height(right) + 1)
        return avl_join_right(left, k, right);
    else if (avl_height(right) > avl_height(left) + 1)
        return avl_join_left(left, k, right);

    AVL_NODE *node = avl_node_new(k);

    node->left = left;
    node->right = right;

    UPDATE_HEIGHT(node);

    return node;
}

static AVL_SPLIT avl_split(AVL_NODE *root, int k)
{
    if (!root)
        return (AVL_SPLIT) { NULL, NULL, false };

    if (k == root->value) {
        AVL_SPLIT result = { root->left, root->right, true };

        free(root);
        return result;
    }

    if (k < root->value) {
        AVL_SPLIT result = avl_split(root->left, k);

        result.right = avl_join(result.right, root->value, root->right);

        free(root);
        return result;
    } else {
        AVL_SPLIT result = avl_split(root->right, k);

        result.left = avl_join(root->left, root->value, result.left);

        free(root);
        return result;
    }
}

static AVL_SPLIT_LAST avl_split_last(AVL_NODE *root)
{
    if (!root->right) {
        AVL_SPLIT_LAST result = { root->left, root->value };

        free(root);
        return result;
    }

    AVL_SPLIT_LAST last = avl_split_last(root->right);

    last.root = avl_join(root->left, root->value, last.root);

    return last;
}

static AVL_NODE *avl_join_no_key(AVL_NODE *left, AVL_NODE *right)
{
    if (!left)
        return right;

    AVL_SPLIT_LAST last = avl_split_last(left);

    return avl_join(last.root, last.value, right);
}

static AVL_NODE *avl_union(AVL_NODE *left, AVL_NODE *right)
{
    if (!left)
        return right;

    if (!right)
        return left;

    AVL_SPLIT split = avl_split(left, right->value);
    AVL_NODE *union_left = avl_union(split.left, right->left);
    AVL_NODE *union_right = avl_union(split.right, right->right);

    return avl_join(union_left, right->value, union_right);
}

static AVL_NODE *avl_intersection(AVL_NODE *left, AVL_NODE *right)
{
    if (!left) {
        avl_free_impl(right);
        return NULL;
    }

    if (!right) {
        avl_free_impl(left);
        return NULL;
    }

    AVL_SPLIT split = avl_split(left, right->value);

    AVL_NODE *intersect_left = avl_intersection(split.left, right->left);
    AVL_NODE *intersect_right = avl_intersection(split.right, right->right);

    if (split.present) {
        AVL_NODE *joined = avl_join(intersect_left, right->value, intersect_right);

        free(right);

        return joined;
    } else {
        free(right);
        return avl_join_no_key(intersect_left, intersect_right);
    }
}

static AVL_NODE *avl_insert_impl(AVL_NODE *root, int value, bool *inserted)
{
    if (!root) {
        root = avl_node_new(value);
        *inserted = true;
    }

    if (value < root->value) {
        root->left = avl_insert_impl(root->left, value, inserted);
    } else if (value > root->value) {
        root->right = avl_insert_impl(root->right, value, inserted);
    }

    UPDATE_HEIGHT(root);

    int bf = avl_height(root->left) - avl_height(root->right);

    if (bf == -2) {
        int bf_right = avl_height(root->right->left) - avl_height(root->right->right);

        if (bf_right <= 0)
            root = avl_rotate_left(root);
        else
            root = avl_rotate_right_left(root);
    }

    if (bf == 2) {
        int bf_left = avl_height(root->left->left) - avl_height(root->left->right);

        if (bf_left >= 0)
            root = avl_rotate_right(root);
        else
            root = avl_rotate_left_right(root);
    }

    return root;
}

bool avl_tree_insert(AVL_TREE *tree, int value)
{
    if (!tree)
        return false;

    bool inserted = false;

    tree->root = avl_insert_impl(tree->root, value, &inserted);

    if (inserted)
        tree->len++;

    return inserted;
}

static AVL_NODE *avl_remove_impl(AVL_NODE *root, int value, bool *removed)
{
    if (!root)
        return NULL;

    if (value == root->value) {
        if (root->left && root->right) {
            /* ... */
        }
    }

    return root;
}

bool avl_tree_remove(AVL_TREE *tree, int value)
{
    if (!tree)
        return false;

    bool removed = false;

    tree->root = avl_remove_impl(tree->root, value, &removed);

    if (removed)
        tree->len--;

    return removed;
}

AVL_NODE *avl_search_impl(AVL_NODE *root, int value)
{
    if (!root)
        return NULL;

    if (value == root->value)
        return root;

    if (value < root->value)
        return avl_search_impl(root->left, value);
    else
        return avl_search_impl(root->right, value);
}

bool avl_tree_search(AVL_TREE *tree, int value)
{
    return avl_search_impl(tree->root, value);
}

size_t avl_tree_len(AVL_TREE *tree)
{
    if (!tree)
        return 0;

    return tree->len;
}

void avl_traverse_impl(AVL_NODE *root, void (*cb)(int value, void *ctx), void *ctx)
{
    if (!root)
        return;

    avl_traverse_impl(root->left, cb, ctx);
    cb(root->value, ctx);
    avl_traverse_impl(root->right, cb, ctx);
}

void avl_tree_traverse(AVL_TREE *tree, void (*cb)(int value, void *ctx), void *ctx)
{
    if (!tree)
        return;

    avl_traverse_impl(tree->root, cb, ctx);
}

AVL_NODE *avl_clone_impl(AVL_NODE *original)
{
    if (!original)
        return NULL;

    AVL_NODE *clone = avl_node_new(original->value);

    clone->left = avl_clone_impl(original->left);
    clone->right = avl_clone_impl(original->right);

    return clone;
}

static void avl_update_len_cb(int value, void *ctx)
{
    (void)value;
    
    size_t *new_len = ctx;
    *new_len = *new_len + 1;
}

AVL_TREE *avl_tree_clone(AVL_TREE *tree)
{
    AVL_TREE *clone = avl_tree_new();

    clone->len = tree->len;
    clone->root = avl_clone_impl(tree->root);

    return clone;
}

AVL_TREE *avl_tree_union(AVL_TREE **a, AVL_TREE **b)
{
    (*a)->root = avl_union((*a)->root, (*b)->root);

    AVL_TREE *result = *a;

    *a = NULL;

    free(*b);
    *b = NULL;

    result->len = 0;
    avl_tree_traverse(result, avl_update_len_cb, &result->len);

    return result;
}

AVL_TREE *avl_tree_intersection(AVL_TREE **a, AVL_TREE **b)
{
    (*a)->root = avl_intersection((*a)->root, (*b)->root);

    AVL_TREE *result = *a;

    *a = NULL;

    free(*b);
    *b = NULL;

    result->len = 0;
    avl_tree_traverse(result, avl_update_len_cb, &result->len);

    return result;
}

void avl_free_impl(AVL_NODE *root)
{
    if (!root)
        return;

    avl_free_impl(root->left);
    avl_free_impl(root->right);

    free(root);
}

void avl_tree_free(AVL_TREE **tree)
{
    if (!tree || !*tree)
        return;

    avl_free_impl((*tree)->root);

    free(*tree);
    *tree = NULL;
}

void swap_max_left(AVL_NODE *swap, AVL_NODE *root, AVL_NODE *before){
    if(swap->right != NULL){
        swap_max_left(swap->right, root, swap);
        return;
    }
    if(root == before){
        before->left = swap->left;
    }else{
        before->right = swap->left;
    }
    root->value = swap->value;
    free(swap);
    swap = NULL;
}

AVL_NODE *avl_tree_remove_aux(AVL_NODE **root, int value){
    if(*root == NULL) return NULL; //se cheguei em um cara NULL, logo, não achei o valor
    else if(value == (*root)->value){ //encontrei o valor desejado

        if((*root)->left == NULL || (*root)->right == NULL){
            //1º e 2º caso, qnd nó encontrado é uma folha ou só tem um filho
            AVL_NODE *aux = *root;

            if((*root)->left == NULL){
                *root = (*root)->right;
            }else{
                *root = (*root)->left;
            }
            free(aux);
            aux = NULL;

        }else{ //caso 3, vc tem ambos os filhos
            swap_max_left((*root)->left, (*root), (*root));
        }
        
    }else if(value < (*root)->value){ 
        //se minha chave procurada é menor que meu node atual
        //logo, vou para a esquerda
        (*root)->left = avl_tree_remove_aux(&(*root)->left, value);

    }else if(value > (*root)->value){
        //se minha chave procurada é maior que meu node atual
        //logo, vou para a direita
        (*root)->right = avl_tree_remove_aux(&(*root)->right, value);
    }

    if(*root != NULL){
        int FB = avl_height((*root)->left) - avl_height((*root)->right);
        if(FB == 2){
            if(avl_height(()))
        }
    }
}

bool avl_tree_remove(AVL_TREE *T, int value){
    if(T == NULL) return false; //árvore inexistente
    T->root = avl_remover_aux(&T->root, value);
    if(T->root == NULL) return false; //se retorna NULL, logo, não consegui tirar esse node, return false
    return true;
}
