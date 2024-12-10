#ifndef RB_H
#define RB_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct rb_tree RB_TREE;

/* Cria uma árvore LLRB, inicialmente vazia */
RB_TREE *rb_tree_new(void);

/* Insere o valor `value` na árvore. Retorna `true` caso a inserção
 * tenha sido bem-sucedida.
 */
bool rb_tree_insert(RB_TREE *, int value);

/* Remove o valor `value` da árvore. Retorna `true` se a remoção 
 * foi bem-sucedida (o nó existia na árvore e foi removido).
 */ 
bool rb_tree_remove(RB_TREE *, int value);

/* Realiza uma busca por `value` na árvore. Retorna `true` caso o valor seja encontrado. */
bool rb_tree_search(RB_TREE *, int value);

/* Retorna o tamanho da árvore (quantidade de elementos) */
size_t rb_tree_len(RB_TREE *);

/* Percorre a árvore em-ordem, chamando `cb` para cada nó e passando como argumentos
 * o valor do nó atual e o valor de `ctx`.
 */
void rb_tree_traverse(RB_TREE *, void (*cb)(int value, void *ctx), void *ctx);

/* Clona a árvore, ou seja, retorna uma outra árvore com a
 * mesma estrutura e mesmos elementos, preservando a árvore original.
 */
RB_TREE *rb_tree_clone(RB_TREE *);

/* Libera espaço ocupado pela árvore e nós. */
void rb_tree_free(RB_TREE **);

#endif

