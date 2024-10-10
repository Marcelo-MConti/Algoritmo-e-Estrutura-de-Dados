#include<stdio.h>
#include<stdlib.h>

int **criar_matriz(int N){
    int **grafo = (int**)malloc(N*sizeof(int*));

    if(grafo == NULL){
        printf("erro na alocação");
        exit(1);
    }

    for(int i = 0; i<N; i++){
        grafo[i] = (int*)malloc(N*sizeof(int));

        if(grafo[i] == NULL){
            printf("erro na alocação");
            exit(1);  
        }
    }
    return grafo;
}

void menor_caminho(int** grafo, int N, int origem){
    origem--;

    int dp[(1<<N) + 2][N+2];
    //dp[i][j]:
    //estou na mask "i" e o ultimo que visitei foi o "j",
    //guardo a menor distância até esse node i, passando por todos que estão ativos na minha mask;

    for(int i = 0; i < (1<<N); i++){
        for(int j = 0; j<N+1; j++){
            dp[i][j] = 1e9;
            //Inicializo minha dp com um valor alto,
            //Para ser mudado na transição da minha dp
        }
    }

    dp[1<<origem][origem] = 0;
    //O ultimo que visitei foi a origem, e só tenho a origem ligada na minha mask
    //logo, a distância que guardo é ZERO

    int mapa[N][1<<N];
    //mapa[i][j]
    //estou no node i, e nessa mask j, tenho como pai
    
    for(int mask = 1; mask < (1<<N); mask++){ //passo por todas as mask
        if((mask & (1<<origem)) == 0) continue; //a origem não está ativa -> dou continue
        for(int pai = 0; pai < N; pai++){ //bruto nos pais que estão ligados na mask
            if((mask & (1<<pai)) == 0) continue; //o pai nem está ativo nessa mask -> dou continue
            for(int filho = 0; filho < N; filho++){ //passo por todos os nodes filhos dele

                if(grafo[pai][filho] == 0 || (mask & (1<<filho))!= 0) continue;
                //não tenho ligação desse pai com esse filho OU já visitei esse filho -> Dou Continue 
                
                if(dp[mask][pai] + grafo[pai][filho] < dp[mask + (1<<filho)][filho]){
                    //Transição da minha dp, vejo se esse novo caminho é melhor do que já tenho guardado

                    dp[mask + (1<<filho)][filho] = dp[mask][pai] + grafo[pai][filho];
                    //atualizo minha dp se achei um cara menor

                    mapa[filho][mask+(1<<filho)] = pai;
                    //atualizo meu mapa, pois encontrei quem é o node que
                    //preciso sair dele, para chegar em uma dada mask
                    //tendo a menor distância possivel
                }
            }
        }
    }

    int resp = 1e9;
    int ultimo;

    //achando a menor distância e o ultimo node que passei pelo meu caminho
    for(int i = 0; i < N; i++){

        if(grafo[i][origem] != 0){
        //tenho uma ligação de volta do ultimo que visitei com o primeiro para fechar o ciclo!
        //vou passar por todos os nodes na mask (2^N - 1), em base 2 : (11...11)
        //logo, representa o a mask que passei por todos os nodes!

            if((dp[(1<<N)-1][i] + grafo[i][origem]) < resp){
            //Acho a menor distância nessa dp que tem todos os bits visitados
            //lembrando que preciso acrescentar o peso da aresta do ultimo com
            //o primeiro, pois no problema ele precisa voltar para a origem 

                resp = dp[(1<<N)-1][i]+ grafo[i][origem];

                //Guardo o meu ultimo no visitado
                ultimo = i;
            } 
        } 
    }

    int eu = ultimo; //começo pelo ultimo node visitado
    int mapa_now = (1<<N) - 1; //começo com meu mapa todo vistado
    
    
    printf("%d\n", origem+1);//printo minha origem
    printf("%d - %d", origem+1, eu+1); //printo minha origem e meu final
    
    //função para printar meu menor caminho
    while(1){

    //a logica se basei em ir de baixo para cima, como guardamos no mapa
    //quem é o melhor node para chegar em uma dada mask para obter o menor caminho
    //logo, basta ir quando a mask está toda preenhida e vem quem chega no node ultimo
    //depois, basta apagar esse node que acabamos de sair da mask e olhar na posição
    //de quem chega no ultimo, que é o que estavamos guardado na matriz anterior

        printf(" - %d", mapa[eu][mapa_now] + 1);
        
        if(mapa[eu][mapa_now] == origem) break;

        int eu_suporte = eu;
        eu = mapa[eu][mapa_now];
        mapa_now = (mapa_now - (1<<eu_suporte));
    
    }

    //printo minha menor distancia encontrada
    printf("\n%d\n", resp);

    //apago minha matriz alocada
    apagar_matriz(grafo, N);
    return;
}

void apagar_matriz(int** grafo, int N){
    for(int i = 0; i<=N; i++){
        free(grafo[i]);
    }
    free(grafo);
}

int main(){
    int N;
    int origem; 
    int m;

    scanf("%d", &N);
    scanf("%d", &origem);
    scanf("%d", &m);

    //criar minha matriz de adjacencia que representa meu grafo
    int **grafo = criar_matriz(N+1);

    for(int i = 0; i < N+1; i++){
        for(int j = 0; j < N+1; j++){
            grafo[i][j] = 0;
            //inicializo todos com zero
        }
    }

    //lendo minhas conexões e meus pesos
    for(int i = 0; i < m; i++){
        int a, b, peso;
        scanf("%d %d %d", &a, &b, &peso);
        grafo[--a][--b] = peso;
        grafo[b][a] = peso;
    }

    //puxo a função menor caminho
    menor_caminho(grafo, N, origem);

}