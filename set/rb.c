#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>

#include "rb.h"

#define BLACK(x) (!(x) || !(x)->is_red)
#define BLACK_NONNULL(x) ((x) && !(x)->is_red)

#define RED(x) ((x) && (x)->is_red)

/* XXX: debugging stuff */
#include <math.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define N_DIG(x) ceil(log10(ABS(x) + 1)) + ((x) < 0)

#define PR_WS(n)                                                                         \
    do {                                                                                 \
        for (int _ = 0; _ < (n); _++)                                                    \
            fputc(' ', stdout);                                                          \
    } while (0)

#define COLOR_RED "\x1b[0;39;41m"
#define COLOR_BLACK "\x1b[0;30;47m"
#define COLOR_RESET "\x1b[0m"

typedef struct rb_node {
    bool is_red;
    int value;

    struct rb_node *left;
    struct rb_node *right;
} RB_NODE;

struct rb_tree {
    RB_NODE *root;
    size_t len;

    /* XXX: debugging stuff */
    int min, max;
};

RB_TREE *rb_tree_new(void)
{
    RB_TREE *tree = malloc(sizeof *tree);

    tree->root = NULL;
    tree->len = 0;

    tree->min = INT_MAX;
    tree->max = INT_MIN;

    return tree;
}

size_t rb_tree_len(RB_TREE *tree)
{
    if (!tree)
        return 0;

    return tree->len;
}

static RB_NODE *rb_node_new(int value)
{
    RB_NODE *node = malloc(sizeof *node);

    node->is_red = true;
    node->value = value;
    node->left = NULL;
    node->right = NULL;

    return node;
}

static void rb_invert(RB_NODE *node)
{
    node->is_red = !node->is_red;

    if (node->left)
        node->left->is_red = !node->left->is_red;

    if (node->right)
        node->right->is_red = !node->right->is_red;
}

static RB_NODE *rb_rotate_left(RB_NODE *node)
{
    RB_NODE *right = node->right;

    node->right = right->left;
    right->left = node;

    right->is_red = right->left->is_red;
    right->left->is_red = true;

    return right;
}

static RB_NODE *rb_rotate_right(RB_NODE *node)
{
    RB_NODE *left = node->left;

    node->left = left->right;
    left->right = node;

    left->is_red = left->right->is_red;
    left->right->is_red = true;

    return left;
}

static RB_NODE *rb_fixup(RB_NODE *root)
{
    if (RED(root->right) && BLACK(root->left))
        root = rb_rotate_left(root);

    if (RED(root->left) && RED(root->left->left))
        root = rb_rotate_right(root);

    if (RED(root->left) && RED(root->right))
        rb_invert(root);

    return root;
}

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

    if (value > tree->max)
        tree->max = value;

    if (value < tree->min)
        tree->min = value;

    if (inserted)
        tree->len++;

    return inserted;
}

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

static RB_NODE *rb_propagate_right(RB_NODE *root)
{
    rb_invert(root);

    if (root->left && RED(root->left->left)) {
        root = rb_rotate_right(root);
        rb_invert(root);
    }

    return root;
}

static void rb_move_min(RB_NODE *node, RB_NODE *subtree)
{
    RB_NODE *min = subtree;

    while (min->left)
        min = min->left;

    node->value = min->value;
}

static RB_NODE *rb_remove_impl(RB_NODE *root, int value, bool *removed)
{
    if (!root)
        return NULL;

    bool remove_min_right = false;

    if (value == root->value) {
        RB_NODE *orphan = NULL;

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

    if (!root)
        return NULL;

    if (value > root->value || remove_min_right) {
        if (RED(root->left))
            root = rb_rotate_right(root);

        if (!remove_min_right && BLACK_NONNULL(root->right) && BLACK(root->right->left))
            root = rb_propagate_right(root);

        if (remove_min_right)
            root->right = rb_remove_impl(root->right, root->value, removed);
        else
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
    tree->root->is_red = false;

    if (removed)
        tree->len--;

    return removed;
}

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

/* XXX: debugging stuff */
static int rb_height(RB_NODE *root)
{
    if (!root)
        return -1;

    int height_left = rb_height(root->left);
    int height_right = rb_height(root->right);

    return MAX(height_left, height_right) + 1;
}

static void rb_print_node(RB_NODE *node, int n_digits)
{
    if (!node) {
        PR_WS(n_digits);
        return;
    }

    if (node->is_red)
        printf(COLOR_RED "%*d" COLOR_RESET, n_digits, node->value);
    else
        printf(COLOR_BLACK "%*d" COLOR_RESET, n_digits, node->value);
}

void rb_tree_print(RB_TREE *tree, void *root)
{
    if (!root)
        root = tree->root;

    if (!tree || !tree->root)
        return;

    int height = rb_height(root);
    size_t max_nodes = (1 << (height + 1)) - 1;

    RB_NODE **deque = malloc(sizeof *deque * max_nodes);

    size_t start = 0;
    size_t end = 0;

    deque[end++] = root;

    int n_digits = MAX(N_DIG(tree->min), N_DIG(tree->max));

    PR_WS(((1 << height) - 1) * n_digits);
    rb_print_node(root, n_digits);

    fputc('\n', stdout);

    for (int i = 1; i <= height; i++) {
        size_t new_start = end;
        size_t new_end = new_start;

        for (int j = 0; j < (end - start + max_nodes) % max_nodes; j++) {
            RB_NODE *current = deque[(start + j) % max_nodes];

            deque[new_end] = current ? current->left : NULL;
            new_end = (new_end + 1) % max_nodes;

            deque[new_end] = current ? current->right : NULL;
            new_end = (new_end + 1) % max_nodes;
        }

        start = new_start;
        end = new_end;

        int n_space_around = ((1 << (height - i)) - 1) * n_digits;

        for (int k = 0; k < (end - start + max_nodes) % max_nodes; k++) {
            if (k == 0)
                PR_WS(n_space_around);
            else
                PR_WS(2 * n_space_around + n_digits);

            rb_print_node(deque[(start + k) % max_nodes], n_digits);
        }

        fputc('\n', stdout);
    }

    free(deque);
}
