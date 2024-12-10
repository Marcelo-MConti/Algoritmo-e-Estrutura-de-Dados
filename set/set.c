#include <stdio.h>
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

struct set_intersection_ctx {
    SET *new;
    SET *old;
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

bool set_insert(SET *set, int value)
{
    if (!set)
        return false;

    switch (set->type) {
        case SET_AVL:
            return avl_tree_insert(set->impl.avl, value);
        case SET_RB:
            return rb_tree_insert(set->impl.rb, value);
    }

    return false;
}

bool set_remove(SET *set, int value)
{
    if (!set)
        return false;

    switch (set->type) {
        case SET_AVL:
            return avl_tree_remove(set->impl.avl, value);
        case SET_RB:
            return rb_tree_remove(set->impl.rb, value);
    }

    return false;
}

bool set_contains(SET *set, int value)
{
    switch (set->type) {
        case SET_AVL:
            return avl_tree_search(set->impl.avl, value);
        case SET_RB:
            return rb_tree_search(set->impl.rb, value);
    }

    return false;
}

static size_t set_len(SET *set)
{
    switch (set->type) {
        case SET_AVL:
            return avl_tree_len(set->impl.avl);
        case SET_RB:
            return rb_tree_len(set->impl.rb);
    }

    return 0;
}

static void set_union_cb(int value, void *ctx)
{
    SET *set = ctx;
    set_insert(set, value);
}

static void set_clone_from(SET *clone, SET *original)
{
    clone->type = original->type;
    
    switch (original->type) {
        case SET_AVL:
            clone->impl.avl = avl_tree_clone(original->impl.avl);
            break;
        case SET_RB:
            clone->impl.rb = rb_tree_clone(original->impl.rb);
            break;
    }
}

SET *set_union(SET *a, SET *b)
{
    if (!a && !b)
        return NULL;
    
    SET *c = malloc(sizeof *c);

    if (!a) {
        set_clone_from(c, b);
        return c;
    }

    if (!b) {
        set_clone_from(c, a);
        return c;
    }

    /* Usa a implementação otimizada, que realiza a união em O(n) */
    if (a->type == b->type && a->type == SET_AVL) {
        AVL_TREE *a_clone = avl_tree_clone(a->impl.avl);
        AVL_TREE *b_clone = avl_tree_clone(b->impl.avl);

        c->type = SET_AVL;
        c->impl.avl = avl_tree_union(&a_clone, &b_clone);

        return c;
    }
    
    /* Fixaremos `a` como o conjunto com a menor quantidade de elementos */
    size_t len_a = set_len(a);
    size_t len_b = set_len(b);

    if (len_b < len_a) {
        SET *tmp = a;
        a = b;
        b = tmp;
    }

    /* A implementação ingênua realiza união em O(n + m * log(n + m)) */
    set_clone_from(c, b);
    
    switch (a->type) {
        case SET_AVL:
            avl_tree_traverse(a->impl.avl, set_union_cb, c);
            break;
        case SET_RB:
            rb_tree_traverse(a->impl.rb, set_union_cb, c);
            break;
    }

    return c;
}

static void set_intersection_cb(int value, void *ctx)
{
    struct set_intersection_ctx *sets = ctx;

    if (set_contains(sets->old, value))
        set_insert(sets->new, value);
}

SET *set_intersection(SET *a, SET *b)
{
    if (!a || !b)
        return NULL;

    /* Fixaremos `a` como o conjunto com a menor quantidade de elementos */
    size_t len_a = set_len(a);
    size_t len_b = set_len(b);

    if (len_b < len_a) {
        SET *tmp = a;
        a = b;
        b = tmp;
    }

    /* Como devemos manter as árvores originais intactas, a implementação de interseção
     * usando `join` e `split` seria mais ineficiente que a implementação "ingênua":
     * Apenas clonar as árvores já levaria O(n + m), enquanto o algoritmo "ingênuo" 
     * de interseção leva O(m * (log(n) + log(m))) = O(m * log(n * m)), com n >= m.
     */
    SET *c = set_new(a->type);

    struct set_intersection_ctx ctx = {
        .old = b,
        .new = c,
    };

    switch (a->type) {
        case SET_AVL:
            avl_tree_traverse(a->impl.avl, set_intersection_cb, &ctx);
            break;
        case SET_RB:
            rb_tree_traverse(a->impl.rb, set_intersection_cb, &ctx);
            break;
    }

    return c;
}

void set_print_cb(int value, void *ctx)
{
    (void)ctx;
    
    printf("%d, ", value);
}

void set_print(SET *set)
{
    if (!set)
        return;

    switch (set->type) {
        case SET_AVL:
            avl_tree_traverse(set->impl.avl, set_print_cb, NULL);
            break;
        case SET_RB:
            rb_tree_traverse(set->impl.rb, set_print_cb, NULL);
            break;
    }

    fputc('\n', stdout);
}

void set_free(SET **set)
{
    if (!set || !*set)
        return;
    
    switch ((*set)->type) {
        case SET_AVL:
            avl_tree_free(&(*set)->impl.avl);
            break;
        case SET_RB:
            rb_tree_free(&(*set)->impl.rb);
            break;
    }

    *set = NULL;
}
