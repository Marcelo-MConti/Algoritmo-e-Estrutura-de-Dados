#include "Grafo.h"

#include<stdio.h>
#include<stdlib.h>

typedef struct no_ NO;

struct no_{
   NO *proximo;
   int chave;
   int peso;
};

/*Estrutura de controle*/
struct grafo_{
   NO *inicio;
   NO *fim;
   int tamanho;
};


GRAFO *grafo_criar(){
   GRAFO *grafo = (GRAFO*) malloc(sizeof(GRAFO));
   
   if(grafo != NULL){
      grafo->inicio = NULL;   
      grafo->fim = NULL;
      grafo->tamanho = 0; 
   }

   return(grafo);
}

bool grafo_inserir(GRAFO *grafo, int chave, int peso){
   if(!grafo_cheia(grafo)){
      NO *no = (NO*) malloc(sizeof(NO));
      if(no == NULL) return false;
      no->chave = chave;
      no->peso = peso;

      no->proximo = NULL;

      if(grafo->inicio == NULL){
         grafo->inicio = no;
         grafo->fim = no;
      }
      else{
         grafo->fim->proximo = no;
         grafo->fim = no;   
      }
      grafo->tamanho++;
      
      return true;
   }
   return false;
}

bool grafo_apagar(GRAFO **grafo){
   if(!grafo_vazia(*grafo)){
      while((*grafo)->inicio != NULL){
         // Tirar no do grafo, deixando-a encadeada.
         NO *no_apagado = (*grafo)->inicio;
         (*grafo)->inicio = (*grafo)->inicio->proximo;
         
         // Apagar no.
         no_apagado->proximo = NULL; 
         free(no_apagado); no_apagado = NULL;
      }
      (*grafo)->fim = NULL;
      free(*grafo); *grafo = NULL; 
      return true;
   }
   return false;
}

int grafo_busca(GRAFO *grafo, int chave){
   if(!grafo_vazia(grafo)){
      NO *busca = grafo->inicio;
      while(busca != NULL){
         if(chave == busca->chave) return(busca->peso);
         busca = busca->proximo;
      }
   }
   return(-1); // Não há ligação
}

int grafo_tamanho(GRAFO *grafo){
   if(grafo != NULL){
      return(grafo->tamanho);
   }
   return(-1);
}

bool grafo_vazia(GRAFO *grafo){
   if(grafo != NULL){
      return(grafo->inicio == NULL);
   }
   return true;
}

bool grafo_cheia(GRAFO *grafo){
   if(grafo != NULL){
      NO *no = (NO*) malloc(sizeof(NO));

      if(no == NULL) return(true);
   
      free(no); no = NULL;
      return(false);
   }
   return(true);
}

void grafo_imprimir(GRAFO *grafo){
   if(!grafo_vazia(grafo)){
      NO *imprimi = grafo->inicio;
      while(imprimi != NULL){
         printf("%d\n", imprimi->chave);
         imprimi = imprimi->proximo;
      }
   }
}

GRAFO **alocar_vetor_grafo(int n){
   GRAFO **vet_grafo = (GRAFO**) malloc(n * sizeof(GRAFO*));
   if(vet_grafo != NULL){
      for(int i = 0; i < n; i++){
         vet_grafo[i] = grafo_criar();
      }
      return vet_grafo;
   }
   return NULL;
}

bool grafo_set_chave(GRAFO* grafo, int chave, int conteudo){

   if(!grafo_vazia(grafo)){
      NO *busca = grafo->inicio;
      while(busca != NULL){
         if(chave == busca->chave){
            busca->peso = conteudo;
            return true;
         }
         busca = busca->proximo;
      }
   }
   return(false); // Não há ligação
}

void menor_caminho(GRAFO **distancia, int origem, int N){

   int dp[(1<<N) + 2][N+2];
   //dp[i][j]:
   //estou na mask "i" e o ultimo que visitei foi o "j",
   //guardo a menor distância até esse node i, passando pela minha mask;

   for(int i = 0; i < (1<<N); i++){
      for(int j = 0; j<N+1; j++){
         //Inicializo minha dp com um valor alto, para ser mudado na transição da minha dp
         dp[i][j] = 1e9; 
      }
   }

   dp[1<<origem][origem] = 0;
   //A distância da minha mask com apenas a origem ligada, sendo a minha origem o node que estou, necessáriamente é zero

   GRAFO **mapa = alocar_vetor_grafo(N+1); 
   //mapa[filho][MASK] = pai 

   for(int mask = 1; mask < (1<<N); mask++){ //passo por todas as mask
      if((mask & (1<<origem)) == 0) continue; //a origem não está ativa -> dou continue
        for(int pai = 0; pai < N; pai++){ //bruto nos pais que estão ligados na mask
            if((mask & (1<<pai)) == 0) continue; //o pai nem está ativo nessa mask -> dou continue
            for(int filho = 0; filho < N; filho++){ //bruto nos filhos dele
               if(grafo_busca(distancia[pai], filho) == -1 || (mask & (1<<filho))!= 0) continue; //não tenho ligação desse pai com esse filho OU já visitei esse filho -> Dou Continue
               if(dp[mask][pai] + grafo_busca(distancia[pai], filho) < dp[mask + (1<<filho)][filho]){ //Transição da minha dp
                  dp[mask + (1<<filho)][filho] = dp[mask][pai] + grafo_busca(distancia[pai], filho); //atualizo minha dp se achei um cara menor

                  if(!grafo_set_chave(mapa[filho], mask+(1<<filho), pai)) grafo_inserir(mapa[filho], mask+(1<<filho), pai);
                  //Se já setei essa pai, na localização (filho, bitmask) da minha matriz eu vou atualizar
                  //Se ainda não setei, eu posso inserir direto na minha matriz
               }
            }
         }
   }

   int resp = 1e9; 
   int ultimo;

   //achando a menor distância e o ultimo node que passei nesse caminho
   for(int i = 0; i < N; i++){ 
      if(grafo_busca(distancia[i], origem) != -1){ 
         //tenho uma ligação de volta do ultimo que visitei com o primeiro para fechar o ciclo!
         //vou passar por todos os nodes na mask (2^N - 1), em base 2 : (11...11)
         //logo, representa o a mask que passei por todos os nodes!

         if((dp[(1<<N)-1][i] + grafo_busca(distancia[i], origem)) < resp){
            //Acho a menor distância nessa dp que tem todos os bits visitados
            //lembrando que preciso acrescentar o peso da aresta do ultimo com o primeiro, pois no problema ele precisa voltar para a origem 

            resp = dp[(1<<N)-1][i]+ grafo_busca(distancia[i], origem);
            //guardo o ultimo visitado
            ultimo = i;
         } 
      }
   }
   
   

   int eu = ultimo;
   int mapa_now = (1<<N) - 1;

   printf("Cidade de Origem: %d\n", origem+1);
   printf("Rota: %d-%d",origem+1,  eu+1);

   //função para printar meu menor caminho
   while(1){

      if(grafo_busca(mapa[eu], mapa_now) == origem){
         printf("-%d\n", grafo_busca(mapa[eu], mapa_now) + 1);
         break;
      }

      printf("-%d", grafo_busca(mapa[eu], mapa_now) + 1);

      int eu_suporte = eu;
      eu = grafo_busca(mapa[eu], mapa_now);
      mapa_now = (mapa_now - (1<<eu_suporte));

   }
   printf("Menor distancia: %d\n", resp);
}

