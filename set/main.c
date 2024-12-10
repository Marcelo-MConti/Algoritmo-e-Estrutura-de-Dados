#include <stdio.h>

#include "set.h"

enum set_operations {
    SET_CONTAINS = 1,
    SET_UNION,
    SET_INTERSECTION,
    SET_REMOVE
};

int main(void)
{
    enum set_type type;
    int len_a, len_b;

    scanf("%d", &type);

    SET *a = set_new(type);
    SET *b = set_new(type);

    scanf("%d %d", &len_a, &len_b);

    for (int i = 0; i < len_a; i++) {
        int n;

        scanf("%d", &n);
        set_insert(a, n);
    }
    
    for (int i = 0; i < len_b; i++) {
        int n;

        scanf("%d", &n);
        set_insert(b, n);
    }

    enum set_operations op;
    scanf("%d", &op);

    SET *c = NULL;

    switch (op) {
        case SET_CONTAINS: {
            int n;
            scanf("%d", &n);
            
            if (set_contains(a, n))
                puts("Pertence.");
            else
                puts("Nao pertence.");
                
            break;
        }
        case SET_UNION:
            c = set_union(a, b);
            set_print(c);
            
            break;
        case SET_INTERSECTION:
            c = set_intersection(a, b);
            set_print(c);
            
            break;
        case SET_REMOVE: {
            int n;
            scanf("%d", &n);
            
            set_remove(a, n);
            set_print(a);
            
            break;
        }
    }

    set_free(&c);
    set_free(&b);
    set_free(&a);
}
