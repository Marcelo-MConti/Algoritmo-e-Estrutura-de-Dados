#ifndef GRAFO_H
    #define GRAFO_H
    #define TAM_MAX 13
    #define INFINITO 100000000

    #include<stdbool.h>

    typedef struct grafo_ GRAFO; 

    GRAFO *grafo_criar();
    bool grafo_inserir(GRAFO *grafo, int chave, int peso);
    bool grafo_apagar(GRAFO **grafo);
    int grafo_busca(GRAFO *grafo, int chave);
    int grafo_tamanho(GRAFO *grafo);
    bool grafo_vazia(GRAFO *grafo);
    bool grafo_cheia(GRAFO *grafo);
    void menor_caminho(GRAFO **distancia, int origem, int tamanho);
    GRAFO **alocar_vetor_grafo(int n);
    bool grafo_set_chave(GRAFO* grafo, int chave, int conteudo);

#endif