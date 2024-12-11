#include <stdio.h>

#include "avl.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define UPDATE_HEIGHT(x)                                                                 \
    do {                                                                                 \
        (x)->height = MAX(avl_height((x)->left), avl_height((x)->right)) + 1;            \
    } while (0)

//node da arvore
typedef struct avl_node {
    //valor armazenado no node
    int value;

    //altura no node, usado para balanceamento
    int height;

    //ponteiros para seus filhos esq e dir
    struct avl_node *left;
    struct avl_node *right;
} AVL_NODE;


//estrutura para dividir a AVL em duas partes
typedef struct avl_split {

    //as duas arvores resultantes da divisão
    AVL_NODE *left;
    AVL_NODE *right;

    //indica se o valor utilizado para dividir estava presente
    bool present;
} AVL_SPLIT;

//usado na divisão da arvore, e pegar o maior valor
typedef struct avl_split_last {
    //arvore resultante apos a remoção do maior valor
    AVL_NODE *root;

    //contem o maior valor removido
    int value;
} AVL_SPLIT_LAST;


//representa a nossa arvore AVL
struct avl_tree {

    //ponteiro para a raiz dessa arvore
    AVL_NODE *root;

    //numero de nos dessa arvore
    size_t len;
};

void avl_free_impl(AVL_NODE *);

/*
criar uma arvore AVL vazia
Inicializando sua raiz como NULL
e seu tamanho como zero
*/
AVL_TREE *avl_tree_new(void)
{
    AVL_TREE *tree = malloc(sizeof *tree);

    tree->root = NULL;
    tree->len = 0;

    return tree;
}

/*
criar um novo node na AVL com o valor fornecido
incializando os ponteiros dos filhos para NULL
e sua altura como 0
*/
static AVL_NODE *avl_node_new(int value)
{
    AVL_NODE *node = malloc(sizeof *node);

    node->value = value;

    node->left = NULL;
    node->right = NULL;

    node->height = 0;

    return node;
}


/*
Retorna a altura desse nó
se for  NULL, retorna -1
*/
static int avl_height(AVL_NODE *node)
{
    if (!node)
        return -1;

    return node->height;
}


/////FUNÇÕES DE ROTAÇÃO/////

/*
Rotação simples para a esquerda:
filho direito do node que está desbalanceado
se torna o novo node raiz, trocando seu filho esquerdo
com o seu pai que estava desbalanceado.
Por fim, atualiza as alturas dos nos envolvidos
*/
static AVL_NODE *avl_rotate_left(AVL_NODE *node)
{
    AVL_NODE *right = node->right;

    node->right = right->left;
    right->left = node;

    UPDATE_HEIGHT(node);
    UPDATE_HEIGHT(right);

    return right;
}

/*
Rotação Simples para a Direita:
Faz o filho esquerdo do node se tornar o novo
node raiz, de forma parecida com a rotação 
esquerda simples.
Por fim, atualiza as alturas
*/
static AVL_NODE *avl_rotate_right(AVL_NODE *node)
{
    AVL_NODE *left = node->left;

    node->left = left->right;
    left->right = node;

    UPDATE_HEIGHT(node);
    UPDATE_HEIGHT(left);

    return left;
}


/*
Rotação dupla esquerda direita:
Uma rotação simples para a esquerda primeiro
seguido de uma rotação simples para a direita
resolvendo o desbalanceamento no lado dirieto do node
*/
static AVL_NODE *avl_rotate_left_right(AVL_NODE *node)
{
    node->left = avl_rotate_left(node->left);
    return avl_rotate_right(node);
}


/*
Rotação dupla direita esquerda:
Uma rotação simples para a direita primeiro
seguido de uma rotação simples para a esquerda
resolvendo o desbalanceamento no lado esquerdo do node
*/
static AVL_NODE *avl_rotate_right_left(AVL_NODE *node)
{
    node->right = avl_rotate_right(node->right);
    return avl_rotate_left(node);
}

/*
Junta duas árvores AVL com um valor intermediario k como node raiz
depois chama duas funções auxiliares, avl_join_left() e avl_join_right()
que vão garantir que a arvore resultante esteja balanceada, ajustando os nodes
*/
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

/*
Se a altura do filho esquerdo de right diferir de no max 1 unidade
maior que o left, cria um novo nó com valor k, que tem left como filho
esquerdo, e o right como filho direito, esse node é anexado como filho 
esquerdo de right.
Verifica o balancemanto e faz as devidas rotações
*/
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

/*
junta duas subarvores AVL com um valor intermediario k como raiz
e garante que o resultando esteja balanceado
*/
static AVL_NODE *avl_join(AVL_NODE *left, int k, AVL_NODE *right)
{
    if (avl_height(left) > avl_height(right) + 1)
        //se lefr for mais alta
        return avl_join_right(left, k, right);

    else if (avl_height(right) > avl_height(left) + 1)
        //se right for mais alta
        return avl_join_left(left, k, right);

    AVL_NODE *node = avl_node_new(k);

    node->left = left;
    node->right = right;

    UPDATE_HEIGHT(node);

    return node;
}

/*
Dividi a arvore em duas, Uma subávore esquerda com valores menores que k
e outra subarvore direita com valores maiores que k e retorna se o valor k
estava presente na árvore original
*/
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


//Remove e retorna o maior valor da árvore 
//utilizado na operação join_no_key
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


/*
Junta duas subarvores AVL, left e right sem valor intermediario
utilizado em operações que um node intermediario não é necessario
*/
static AVL_NODE *avl_join_no_key(AVL_NODE *left, AVL_NODE *right)
{
    if (!left)
        return right;

    AVL_SPLIT_LAST last = avl_split_last(left);

    return avl_join(last.root, last.value, right);
}


/*
Realiaza a união de duas arvores AVL, utilizando a operação
avl_split() para dividir e combinar de forma recursiva as árvores
*/
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

/*
Operação de encontrar a interseção de duas arvores AVL
descarta os valores que não estão presentees em ambas as árvores
*/
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
    //caso a arvore esteja, vazia ele cria e coloca o novo node
    if (!root) {
        root = avl_node_new(value);
        *inserted = true;
    }

    //Localiza a posição do novo valor
    if (value < root->value) {
        root->left = avl_insert_impl(root->left, value, inserted);
    } else if (value > root->value) {
        root->right = avl_insert_impl(root->right, value, inserted);
    }

    //atualiza a altura da arvore
    UPDATE_HEIGHT(root);

    //Verifica o fator de balancemaneto e realiza as rotações necessarias
    //para que a arvore permaneça balanceada
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


/*
insere um valor na arvore AVL, utilizando a função
avl_insert_impl() para localizar a posição correta do node
e ajustar o balanceamento na árvore, atualizando sua altura
no final
*/
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

/*
Procura um valor na árvore AVL, utiliza avl_search_impl()
para encontrar de forma recursiva
*/
bool avl_tree_search(AVL_TREE *tree, int value)
{
    return avl_search_impl(tree->root, value);
}

//retorna o tamanho da árvore
size_t avl_tree_len(AVL_TREE *tree)
{
    if (!tree)
        return 0;

    return tree->len;
}


/*
Realiza um percurso em ordem na AVL realizando a operação (cb)
para cada valor encontrado
*/
void avl_traverse_impl(AVL_NODE *root, void (*cb)(int value, void *ctx), void *ctx)
{
    if (!root)
        return;

    avl_traverse_impl(root->left, cb, ctx);
    cb(root->value, ctx);
    avl_traverse_impl(root->right, cb, ctx);
}

//Utiliza a função avl_traverse_impl() para percorrer a arvore e realizar
//a operação callback (cb) na arvore
void avl_tree_traverse(AVL_TREE *tree, void (*cb)(int value, void *ctx), void *ctx)
{
    if (!tree)
        return;

    avl_traverse_impl(tree->root, cb, ctx);
}


/*
função para clonar uma arvore AVL, sendo avl_clone_impl()
utilizando para clonar de forma recursiva os nodes
*/
AVL_NODE *avl_clone_impl(AVL_NODE *original)
{
    if (!original)
        return NULL;

    AVL_NODE *clone = avl_node_new(original->value);

    clone->left = avl_clone_impl(original->left);
    clone->right = avl_clone_impl(original->right);

    return clone;
}

/*
Usado para atualizar o comprimento (len) da árvore durante um percurso
incrementando o contador de tamanho durante o percurso
*/
static void avl_update_len_cb(int value, void *ctx)
{
    (void)value;
    
    size_t *new_len = ctx;
    *new_len = *new_len + 1;
}

//cria a copia da AVL e instancia a avl_clone_impl()
AVL_TREE *avl_tree_clone(AVL_TREE *tree)
{
    AVL_TREE *clone = avl_tree_new();

    clone->len = tree->len;
    clone->root = avl_clone_impl(tree->root);

    return clone;
}


/*
Calcula a UNIAO de duas Arvores AVL, utilizando a avl_union()
modificando o ponteiro (a), depois de colocar a resposta em 
result, libera a memoria de ambas as arvores, depois percorre 
a arvore para calcular len
*/
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


/*
Modifica a primeira arvore (a) para conter os elementos da interseção e
librera a memoria da segunda arvore (b)
*/
AVL_TREE *avl_tree_intersection(AVL_TREE **a, AVL_TREE **b)
{
    (*a)->root = avl_intersection((*a)->root, (*b)->root);
    //calcula a interseção das duas arvores, atulizando (a)

    AVL_TREE *result = *a;

    *a = NULL;

    free(*b);
    *b = NULL;

    result->len = 0;
    //percorre a arvore resultante para calcular o valor de len
    avl_tree_traverse(result, avl_update_len_cb, &result->len);

    //retorna a arvore resultante da interseção
    return result;
}

//operação recursiva para apagar a arvore
void avl_free_impl(AVL_NODE *root)
{
    if (!root)
        return;

    avl_free_impl(root->left);
    avl_free_impl(root->right);

    free(root);
}

//função para apagar a arvore
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
        //se ainda não cheguei em um node que não tem filho pela direita, ou seja,
        //ainda não cheguei no maior node que está na esquerda
        //continuo minha recursão
        swap_max_left(swap->right, root, swap);
        return;
    }

    if(root == before){ 
        //se minha raiz é igual o meu anterior
        //logo, eu já chamei a função e cheguei no caso em que meu filho esquerdo
        //já é o meu maior filho da esquerda

        before->left = swap->left;
        //troco a ligação da minha raiz com o filho esquerdo de quem quero trocar
    }else{
        //caso não caia no caso especificado anteriormente

        before->right = swap->left;
        //troco o filho do meu node anterior de quem eu vou trocar
        //com o filho esquerdo de quem quero trocar

    }
    //mudo os valores 
    root->value = swap->value;
    //dou free no node que quero tirar
    free(swap);
    swap = NULL;
}

AVL_NODE *avl_tree_remove_aux(AVL_NODE **root, int value, bool *remove){
    if(*root == NULL){ 
        return NULL; //se cheguei em um cara NULL, logo, retorno NULL

    }else if(value == (*root)->value){ //encontrei o valor desejado!

        *remove = true; //coloco minha FLAG para true

        if((*root)->left == NULL || (*root)->right == NULL){
            //1º ou 2º caso, qnd nó encontrado é uma folha ou só tem um unico filho respectivamente
            AVL_NODE *aux = *root;

            if((*root)->left == NULL){
                //se meu filho esquerdo for NULL
                //Logo, toco essa raiz pelo o filho direito dela
                *root = (*root)->right;
            }else{
                //se meu filho direito for NULL
                //Logo, troco essa raiz pelo meu filho esquerdo
                *root = (*root)->left;
            }
            //dou free nessa raiz, pelo aux
            free(aux);
            aux = NULL;

        }else{ 
            //caso 3, o no que preciso retirar tem ambos os filhos
            //vou trocar ele pelo o maior da esquerda e depois retirar ele
            swap_max_left((*root)->left, (*root), (*root));
        }
        
    }else if(value < (*root)->value){ 

        //se minha chave procurada é menor que meu node atual
        //logo, vou para a esquerda
        (*root)->left = avl_tree_remove_aux(&(*root)->left, value, remove);

    }else if(value > (*root)->value){

        //se minha chave procurada é maior que meu node atual
        //logo, vou para a direita
        (*root)->right = avl_tree_remove_aux(&(*root)->right, value, remove);
    }

    if(*root != NULL){
        //calculo do fator de balanceamento do nó
        int FB = avl_height((*root)->left) - avl_height((*root)->right);

        if(FB == 2){ //se meu fator de balanceamento for 2 (rotação direita)

            if((avl_height(((*root)->left)->left) -  avl_height(((*root)->left)->right)) >= 0)
                //se o fator de balancemanto do meu filho esquero, tiver o mesmo sinal que eu
                //logo, preciso fazer uma rotação simples para direita
                *root = avl_rotate_right(*root);

            else
                //caso tenha sinal oposto, então eu preciso fazer uma rotação dupla
                //uma rotação esquerda direita
                *root = avl_rotate_left_right(*root);
        }

        if(FB == -2){
            //se meu fator de balanceamnto for igual a -2, 
            //logo, posso fazer uma rotação simples para a esquerda
            //ou fazer uma rotação dupla, direita_esquerda

            if((avl_height(((*root)->right)->left) -  avl_height(((*root)->right)->right)) <= 0)
                //se o fator de balancemanto do meu filho direito, tiver o mesmo sinal que eu
                //logo, preciso fazer uma rotação simples para esquerda
                *root = avl_rotate_left(*root);

            else
                //caso tenha sinal oposto, então eu preciso fazer uma rotação dupla
                //uma rotação direita esquerda
                *root = avl_rotate_right_left(*root);
        }
            
    }
    return *root;
}

bool avl_tree_remove(AVL_TREE *T, int value){
    if(T == NULL) return false; //árvore inexistente
    bool remove = false; //minha flag, inicializo minha flag como false no começo
    T->root = avl_tree_remove_aux(&T->root, value, &remove);

    if (remove) T->len--;
    return remove; //minha resposta é minha flag
}
