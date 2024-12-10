#include <stdlib.h>
#include <stdbool.h>

#include "rb.h"

#define BLACK(x) (!(x) || !(x)->is_red)
#define BLACK_NONNULL(x) ((x) && !(x)->is_red)

#define RED(x) ((x) && (x)->is_red)

typedef struct rb_node {
    bool is_red;
    int value;

    struct rb_node *left;
    struct rb_node *right;
} RB_NODE;

struct rb_tree {
    RB_NODE *root;
    size_t len;
};

RB_TREE *rb_tree_new(void)
{
    RB_TREE *tree = malloc(sizeof *tree);

    tree->root = NULL;
    tree->len = 0;

    return tree;
}

size_t rb_tree_len(RB_TREE *tree)
{
    if (!tree)
        return 0;

    return tree->len;
}

/* Cria um nó, inicialmente vermelho */  
static RB_NODE *rb_node_new(int value)
{
    RB_NODE *node = malloc(sizeof *node);

    node->is_red = true;
    node->value = value;
    node->left = NULL;
    node->right = NULL;

    return node;
}

/* Inverte as cores do nó `node` e de seus filhos */
static void rb_invert(RB_NODE *node)
{
    node->is_red = !node->is_red;

    if (node->left)
        node->left->is_red = !node->left->is_red;

    if (node->right)
        node->right->is_red = !node->right->is_red;
}

/*
 * Roda o nó `node` para a esquerda. Essa função assume que
 * a operação rotação para a esquerda só será usada quando o 
 * filho direito tiver aresta incidente vermelha.
 */
static RB_NODE *rb_rotate_left(RB_NODE *node)
{
    RB_NODE *right = node->right;

    node->right = right->left;
    right->left = node;

    right->is_red = right->left->is_red;
    right->left->is_red = true;

    return right;
}

/* Roda o nó `node` para a direita */
static RB_NODE *rb_rotate_right(RB_NODE *node)
{
    RB_NODE *left = node->left;

    node->left = left->right;
    left->right = node;

    left->is_red = left->right->is_red;
    left->right->is_red = true;

    return left;
}

/* Realiza as correções necessárias para manter as condições
 * de balanceamento da árvore LLRB, usada na inserção e remoção.
 * É importante notar que a ordem em que as verificações são feitas
 * importa, pois a corrigir a violação de uma regra pode causar a
 * violação de outra regra.
 */
static RB_NODE *rb_fixup(RB_NODE *root)
{
    // Arestas vermelhas à direita não são permitidas: uma rotação esquerda
    // irá mover essa aresta para a esquerda.
    if (RED(root->right) && BLACK(root->left))
        root = rb_rotate_left(root);

    // Arestas vermelhas consecutivas não são permitidas: uma rotação direita
    // irá "mover" uma das arestas para a direita.
    if (RED(root->left) && RED(root->left->left))
        root = rb_rotate_right(root);

    // Dois filhos vermelhos não são permitidos: uma inversão de cores irá
    // propagar essa aresta vermelha até a raiz, garantindo que a condição
    // de balanceamento negro perfeito não é violada. 
    if (RED(root->left) && RED(root->right))
        rb_invert(root);

    return root;
}

/* Insere o valor `value` na árvore com raiz em `root`. Atribui `true` a 
 * `*inserted` caso a inserção tenha sido bem sucedida. A inserção irá falhar
 * caso um nó com valor `value` já exista na árvore.
 */
static RB_NODE *rb_insert_impl(RB_NODE *root, int value, bool *inserted)
{
    if (!root) {
        root = rb_node_new(value);
        *inserted = true;
    }

    if (value < root->value)
        root->left = rb_insert_impl(root->left, value, inserted);
    else if (value > root->value)
        root->right = rb_insert_impl(root->right, value, inserted);

    return rb_fixup(root);
}

bool rb_tree_insert(RB_TREE *tree, int value)
{
    if (!tree)
        return false;

    bool inserted = false;

    tree->root = rb_insert_impl(tree->root, value, &inserted);
    tree->root->is_red = false;

    if (inserted)
        tree->len++;

    return inserted;
}

/* Propaga uma aresta vermelha para o filho esquerdo de `root`.
 * Usado na remoção.
 */
static RB_NODE *rb_propagate_left(RB_NODE *root)
{
    rb_invert(root);

    if (root->right && RED(root->right->left)) {
        root->right = rb_rotate_right(root->right);
        root = rb_rotate_left(root);

        rb_invert(root);
    }

    return root;
}

/* Propaga uma aresta vermelha para o filho direito de `root`.
 * Usado na remoção.
 */
static RB_NODE *rb_propagate_right(RB_NODE *root)
{
    rb_invert(root);

    if (root->left && RED(root->left->left)) {
        root = rb_rotate_right(root);
        rb_invert(root);
    }

    return root;
}

/* Copia o valor do menor nó da árvore com raiz em `subtree` para o nó `node`. 
 * Usado na remoção.
 */
static void rb_move_min(RB_NODE *node, RB_NODE *subtree)
{
    RB_NODE *min = subtree;

    while (min->left)
        min = min->left;

    node->value = min->value;
}

/* Remove um nó com valor `value` da árvore com raiz em `root`, se existir. */ 
static RB_NODE *rb_remove_impl(RB_NODE *root, int value, bool *removed)
{
    if (!root)
        return NULL;

    bool remove_min_right = false;

    if (value == root->value) {
        RB_NODE *orphan = NULL;

        // Essa mesma função trata todos os casos: a flag remove_min_right é
        // usada para que passemos a buscar o menor nó da direita para removê-lo,
        // em vez do nó que o usuário queria remover originalmente, uma vez que
        // esse nó (o menor da direita) já foi copiado para a posição do nó com
        // o valor a ser removido.
        if (root->right && root->left) {
            rb_move_min(root, root->right);
            remove_min_right = true;
        } else if (root->right) {
            orphan = root;
            root = root->right;
        } else {
            orphan = root;
            root = root->left;
        }

        if (orphan) {
            if (root)
                root->is_red = orphan->is_red;

            *removed = true;
            free(orphan);
        }
    }

    // `root` pode se tornar `NULL` após a remoção efetuada acima
    if (!root)
        return NULL;

    if (value > root->value || remove_min_right) {
        if (remove_min_right)
            value = root->value;

        // Caso haja uma aresta vermelha à esquerda e a busca deve prosseguir à direita,
        // devemos "aproveitar" essa aresta, realizando uma rotação para a direita.
        if (RED(root->left))
            root = rb_rotate_right(root);

        if (BLACK_NONNULL(root->right) && BLACK(root->right->left))
            root = rb_propagate_right(root);

        root->right = rb_remove_impl(root->right, value, removed);
    } else if (value < root->value) {
        if (BLACK_NONNULL(root->left) && BLACK(root->left->left))
            root = rb_propagate_left(root);

        root->left = rb_remove_impl(root->left, value, removed);
    }

    return rb_fixup(root);
}

bool rb_tree_remove(RB_TREE *tree, int value)
{
    if (!tree)
        return false;

    bool removed = false;

    tree->root = rb_remove_impl(tree->root, value, &removed);

    // A raiz pode não existir mais após uma remoção
    if (tree->root)
        tree->root->is_red = false;

    if (removed)
        tree->len--;

    return removed;
}

/* Realiza uma busca por `value` na árvore com raiz em `root`. Retorna o nó
 * caso seja encontrado, caso contrário, retorna `NULL`.
 */
static RB_NODE *rb_search_impl(RB_NODE *root, int value)
{
    if (!root)
        return NULL;

    if (value > root->value)
        return rb_search_impl(root->right, value);
    else if (value < root->value)
        return rb_search_impl(root->left, value);
    else
        return root;
}

bool rb_tree_search(RB_TREE *tree, int value)
{
    if (!tree)
        return false;

    return rb_search_impl(tree->root, value);
}

static void rb_traverse_impl(RB_NODE *root, void (*cb)(int, void *), void *ctx)
{
    if (!root)
        return;

    rb_traverse_impl(root->left, cb, ctx);
    cb(root->value, ctx);
    rb_traverse_impl(root->right, cb, ctx);
}

void rb_tree_traverse(RB_TREE *tree, void (*cb)(int, void *), void *ctx)
{
    if (!tree)
        return;

    rb_traverse_impl(tree->root, cb, ctx);
}

RB_NODE *rb_clone_impl(RB_NODE *original)
{
    if (!original)
        return NULL;

    RB_NODE *clone = rb_node_new(original->value);

    clone->left = rb_clone_impl(original->left);
    clone->right = rb_clone_impl(original->right);

    return clone;
}

RB_TREE *rb_tree_clone(RB_TREE *tree)
{
    RB_TREE *clone = rb_tree_new();

    clone->len = tree->len;
    clone->root = rb_clone_impl(tree->root);

    return clone;
}

static void rb_free_impl(RB_NODE *root)
{
    if (!root)
        return;

    rb_free_impl(root->left);
    rb_free_impl(root->right);

    free(root);
}

void rb_tree_free(RB_TREE **tree)
{
    if (!tree || !*tree)
        return;

    rb_free_impl((*tree)->root);

    free(*tree);
    *tree = NULL;
}
