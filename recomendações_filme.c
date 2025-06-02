#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>

#define MAX_FILMES 300
#define MAX_GENEROS 50
#define MAX_GENEROS_POR_FILME 5
#define MAX_STR 128

typedef struct {
    char nome[MAX_STR];
    char ano[10];
    char classificacao[10];
    char duracao[20];
    char generos[MAX_GENEROS_POR_FILME][30];
    int qtdGeneros;
} Filme;

typedef struct {
    char nome[30];
    int filmes[MAX_FILMES];
    int qtdFilmes;
} Genero;

Filme filmes[MAX_FILMES];
int qtdFilmes = 0;

Genero generos[MAX_GENEROS];
int qtdGeneros = 0;

int matrizAdj[MAX_FILMES][MAX_FILMES] = {0};

typedef struct No {
    int id;
    struct No* prox;
} No;

No* listaAdj[MAX_FILMES] = {NULL};

void limpar_str(char* str) {
    // Remove espaços à direita
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1]))
        str[--len] = '\0';

    // Remove espaços à esquerda
    int start = 0;
    while (isspace((unsigned char)str[start]))
        start++;

    if (start > 0)
        memmove(str, str + start, strlen(str + start) + 1);
}

int obter_ou_criar_genero(char* nome) {
    limpar_str(nome);  // <--- limpa espaços/sujeiras
    for (int i = 0; i < qtdGeneros; i++) {
        if (strcasecmp(generos[i].nome, nome) == 0)
            return i;
    }
    strcpy(generos[qtdGeneros].nome, nome);
    generos[qtdGeneros].qtdFilmes = 0;
    return qtdGeneros++;
}


void adicionar_filme_ao_genero(int generoID, int filmeID) {
    Genero* g = &generos[generoID];
    g->filmes[g->qtdFilmes++] = filmeID;
}

void conectar_filmes(int a, int b) {
    matrizAdj[a][b] = 1;
    matrizAdj[b][a] = 1;

    No* novo = malloc(sizeof(No));
    novo->id = b;
    novo->prox = listaAdj[a];
    listaAdj[a] = novo;

    novo = malloc(sizeof(No));
    novo->id = a;
    novo->prox = listaAdj[b];
    listaAdj[b] = novo;
}

int dividir_csv(char* linha, char* campos[], int max_campos) {
    int i = 0;
    char* ptr = linha;
    while (*ptr && i < max_campos) {
        if (*ptr == '"') {
            ptr++;
            campos[i++] = ptr;
            while (*ptr && !(*ptr == '"' && (*(ptr + 1) == ',' || *(ptr + 1) == '\0')))
                ptr++;
            *ptr = '\0';
            ptr += 2;
        } else {
            campos[i++] = ptr;
            while (*ptr && *ptr != ',') ptr++;
            if (*ptr == ',') *ptr++ = '\0';
        }
    }
    return i;
}

void ler_csv(const char* nomeArquivo) {
    FILE* arq = fopen(nomeArquivo, "r");
    if (!arq) {
        perror("Erro ao abrir CSV");
        exit(1);
    }

    char linha[1024];
    fgets(linha, 1024, arq);

    while (fgets(linha, 1024, arq) && qtdFilmes < MAX_FILMES) {
        char* campos[15];
        int campo = dividir_csv(linha, campos, 15);

        if (campo < 6) continue;

        Filme* f = &filmes[qtdFilmes];
        strncpy(f->nome, campos[1], MAX_STR - 1);
        f->nome[MAX_STR - 1] = '\0';
        limpar_str(f->nome);

        f->qtdGeneros = 0;
        char* genero_str = campos[4];
        limpar_str(genero_str);

        char* gtoken = strtok(genero_str, ",");
while (gtoken && f->qtdGeneros < MAX_GENEROS_POR_FILME) {
    limpar_str(gtoken);

    // Remover aspas finais e iniciais, se existirem
    size_t len = strlen(gtoken);
    if (len > 0 && gtoken[len - 1] == '"') {
        gtoken[len - 1] = '\0';
    }
    if (gtoken[0] == '"') {
        memmove(gtoken, gtoken + 1, strlen(gtoken));
    }

    strcpy(f->generos[f->qtdGeneros], gtoken);
    int gID = obter_ou_criar_genero(gtoken);
    adicionar_filme_ao_genero(gID, qtdFilmes);
    f->qtdGeneros++;
    gtoken = strtok(NULL, ",");
}


        qtdFilmes++;
    }
    fclose(arq);

    for (int i = 0; i < qtdFilmes; i++) {
        for (int j = i + 1; j < qtdFilmes; j++) {
            for (int k = 0; k < filmes[i].qtdGeneros; k++) {
                for (int l = 0; l < filmes[j].qtdGeneros; l++) {
                    if (strcmp(filmes[i].generos[k], filmes[j].generos[l]) == 0) {
                        conectar_filmes(i, j);
                        goto proximo;
                    }
                }
            }
            proximo:;
        }
    }
}

void mostrar_generos() {
    printf("\nGêneros disponíveis:\n");
    for (int i = 0; i < qtdGeneros; i++) {
        printf(" %2d. %s\n", i + 1, generos[i].nome);
    }
}

void inserir_filme_manual() {
    if (qtdFilmes >= MAX_FILMES) {
        printf("\nLimite de filmes atingido.\n");
        return;
    }

    Filme* f = &filmes[qtdFilmes];

    printf("\nDigite o nome do filme: ");
    fgets(f->nome, MAX_STR, stdin);
    limpar_str(f->nome);

    printf("Ano: ");
    fgets(f->ano, sizeof(f->ano), stdin);
    limpar_str(f->ano);

    printf("Classificação indicativa (ex: PG-13): ");
    fgets(f->classificacao, sizeof(f->classificacao), stdin);
    limpar_str(f->classificacao);

    printf("Duração (ex: 2h 10m): ");
    fgets(f->duracao, sizeof(f->duracao), stdin);
    limpar_str(f->duracao);

    mostrar_generos();
    do {
        printf("Quantos gêneros (1 a %d): ", MAX_GENEROS_POR_FILME);
        scanf("%d", &f->qtdGeneros);
        getchar();
        if (f->qtdGeneros < 1 || f->qtdGeneros > MAX_GENEROS_POR_FILME)
            printf("Número inválido. Tente novamente.\n");
    } while (f->qtdGeneros < 1 || f->qtdGeneros > MAX_GENEROS_POR_FILME);

    for (int i = 0; i < f->qtdGeneros; i++) {
        int opc;
        do {
            printf("Escolha o código do gênero #%d: ", i + 1);
            scanf("%d", &opc);
            getchar();
            opc--;
            if (opc >= 0 && opc < qtdGeneros) {
                strcpy(f->generos[i], generos[opc].nome);
                adicionar_filme_ao_genero(opc, qtdFilmes);
                break;
            } else {
                printf("Código inválido. Tente novamente.\n");
            }
        } while (1);
    }

    for (int i = 0; i < qtdFilmes; i++) {
        for (int k = 0; k < f->qtdGeneros; k++) {
            for (int l = 0; l < filmes[i].qtdGeneros; l++) {
                if (strcmp(f->generos[k], filmes[i].generos[l]) == 0) {
                    conectar_filmes(i, qtdFilmes);
                    goto conectado;
                }
            }
        }
        conectado:;
    }

FILE* arq = fopen("IMDB Top 250 Movies.csv", "a");
    if (arq) {
        fprintf(arq, "\"%s\",\"%s\",\"%s\",\"%s\",\"",
                f->ano, f->nome, f->classificacao, f->duracao);
        for (int i = 0; i < f->qtdGeneros; i++) {
            fprintf(arq, "%s%s", f->generos[i], i < f->qtdGeneros - 1 ? "," : "");
        }
        fprintf(arq, "\"\n");
        fclose(arq);
    } else {
        printf("\nErro ao abrir o arquivo para salvar o novo filme.\n");
    }

    qtdFilmes++;
}

void recomendar_por_genero(char* entrada) {
    int generos_usuario[MAX_GENEROS];
    int qtd_generos_usuario = 0;

    char entrada_copia[128];
    strncpy(entrada_copia, entrada, sizeof(entrada_copia));
    entrada_copia[sizeof(entrada_copia) - 1] = '\0';

    char* token = strtok(entrada_copia, ",");
    while (token && qtd_generos_usuario < MAX_GENEROS) {
        limpar_str(token);
int encontrado = 0;
for (int i = 0; i < qtdGeneros; i++) {
    char genero_limpo[30];
    strncpy(genero_limpo, generos[i].nome, sizeof(genero_limpo));
    genero_limpo[sizeof(genero_limpo) - 1] = '\0';
    limpar_str(genero_limpo);  // limpa possíveis sujeiras no vetor também

    if (strcasecmp(token, genero_limpo) == 0) {
        generos_usuario[qtd_generos_usuario++] = i;
        encontrado = 1;
        break;
    }
}

        if (!encontrado) {
            printf("Gênero inválido: '%s'. Veja a lista abaixo:\n", token);
            mostrar_generos();
            return;
        }
        token = strtok(NULL, ",");
    }

    printf("\nFilmes recomendados com TODOS os gêneros fornecidos:\n");
    int encontrou = 0;
    for (int i = 0; i < qtdFilmes; i++) {
        int possuiTodos = 1;
        for (int j = 0; j < qtd_generos_usuario; j++) {
            int ok = 0;
            for (int k = 0; k < filmes[i].qtdGeneros; k++) {
                if (strcasecmp(filmes[i].generos[k], generos[generos_usuario[j]].nome) == 0) {
                    ok = 1;
                    break;
                }
            }
            if (!ok) {
                possuiTodos = 0;
                break;
            }
        }
        if (possuiTodos) {
            printf(" - %s\n", filmes[i].nome);
            encontrou = 1;
        }
    }

    if (!encontrou) {
        printf("Nenhum filme encontrado com todos os gêneros informados.\n");
    }
}

void bfs(int inicio) {
    int visitado[MAX_FILMES] = {0};
    int fila[MAX_FILMES];
    int ini = 0, fim = 0;

    typedef struct {
        int id;
        int intersecao;
    } FilmeRelacao;

    FilmeRelacao relacionados[MAX_FILMES];
    int qtdRelacionados = 0;

    fila[fim++] = inicio;
    visitado[inicio] = 1;

    while (ini < fim) {
        int atual = fila[ini++];

        if (atual != inicio) {
            int intersecao = 0;
            for (int i = 0; i < filmes[inicio].qtdGeneros; i++) {
                for (int j = 0; j < filmes[atual].qtdGeneros; j++) {
                    if (strcasecmp(filmes[inicio].generos[i], filmes[atual].generos[j]) == 0) {
                        intersecao++;
                        break;
                    }
                }
            }
            if (intersecao > 0) {
                relacionados[qtdRelacionados].id = atual;
                relacionados[qtdRelacionados].intersecao = intersecao;
                qtdRelacionados++;
            }
        }

        for (No* viz = listaAdj[atual]; viz; viz = viz->prox) {
            if (!visitado[viz->id]) {
                visitado[viz->id] = 1;
                fila[fim++] = viz->id;
            }
        }
    }

    for (int i = 0; i < qtdRelacionados - 1; i++) {
        for (int j = i + 1; j < qtdRelacionados; j++) {
            if (relacionados[j].intersecao > relacionados[i].intersecao) {
                FilmeRelacao temp = relacionados[i];
                relacionados[i] = relacionados[j];
                relacionados[j] = temp;
            }
        }
    }

    printf("\nFilmes relacionados a '%s' (ordenados por gêneros em comum):\n", filmes[inicio].nome);
    for (int i = 0; i < qtdRelacionados; i++) {
        printf(" - %s (%d gênero(s) em comum)\n", filmes[relacionados[i].id].nome, relacionados[i].intersecao);
    }
}

void menu_busca_por_filme() {
    if (qtdFilmes == 0) {
        printf("\nNenhum filme disponível para busca.\n");
        return;
    }

    printf("\nEscolha um filme pelo código:\n");
    for (int i = 0; i < qtdFilmes; i++) {
        printf(" %3d. %s\n", i + 1, filmes[i].nome);
    }

    printf("Digite o código: ");
    int cod;
    if (scanf("%d", &cod) != 1 || cod < 1 || cod > qtdFilmes) {
        printf("Código inválido.\n");
        while (getchar() != '\n');
        return;
    }

    getchar();
    bfs(cod - 1);
}

void calcular_rota_entre_filmes() {
    if (qtdFilmes < 2) {
        printf("É necessário ao menos 2 filmes para calcular rota.\n");
        return;
    }

    printf("\nFilmes disponíveis:\n");
    for (int i = 0; i < qtdFilmes; i++) {
        printf(" %3d. %s\n", i + 1, filmes[i].nome);
    }

    int origem, destino;
    printf("\nDigite o código do filme de origem: ");
    if (scanf("%d", &origem) != 1 || origem < 1 || origem > qtdFilmes) {
        printf("Código inválido.\n");
        while (getchar() != '\n');
        return;
    }

    printf("Digite o código do filme de destino: ");
    if (scanf("%d", &destino) != 1 || destino < 1 || destino > qtdFilmes) {
        printf("Código inválido.\n");
        while (getchar() != '\n');
        return;
    }

    origem--; destino--;
    int visitado[MAX_FILMES] = {0};
    int anterior[MAX_FILMES];
    for (int i = 0; i < MAX_FILMES; i++) anterior[i] = -1;

    int fila[MAX_FILMES], ini = 0, fim = 0;
    fila[fim++] = origem;
    visitado[origem] = 1;

    while (ini < fim) {
        int atual = fila[ini++];
        if (atual == destino) break;

        for (No* viz = listaAdj[atual]; viz; viz = viz->prox) {
            if (!visitado[viz->id]) {
                visitado[viz->id] = 1;
                anterior[viz->id] = atual;
                fila[fim++] = viz->id;
            }
        }
    }

    if (!visitado[destino]) {
        printf("\nNão existe rota entre '%s' e '%s'.\n", filmes[origem].nome, filmes[destino].nome);
        return;
    }

    int caminho[MAX_FILMES], tam = 0;
    for (int v = destino; v != -1; v = anterior[v])
        caminho[tam++] = v;

    printf("\nRota entre '%s' e '%s':\n", filmes[origem].nome, filmes[destino].nome);
    for (int i = tam - 1; i >= 0; i--) {
        printf(" -> %s\n", filmes[caminho[i]].nome);
    }
}


int main() {
    setlocale(LC_ALL, "Portuguese");
    ler_csv("IMDB Top 250 Movies.csv");

    int opc;
    do {
        printf("\nMenu:\n");
        printf("1. Mostrar gêneros\n");
        printf("2. Inserir novo filme\n");
        printf("3. Recomendar por gênero\n");
        printf("4. Buscar filmes relacionados (BFS)\n");
        printf("5. Calcular rota entre dois filmes\n");
        printf("0. Sair\n");
        printf("Escolha: ");
        if (scanf("%d", &opc) != 1) {
            printf("Opção inválida. Digite um número.\n");
            while (getchar() != '\n');
            continue;
        }
        getchar();

        if (opc == 1) mostrar_generos();
        else if (opc == 2) inserir_filme_manual();
        else if (opc == 3) {
            char entrada[128];
            printf("Digite os gêneros separados por vírgula: ");
            fgets(entrada, sizeof(entrada), stdin);
            limpar_str(entrada);
            recomendar_por_genero(entrada);
        } else if (opc == 4) menu_busca_por_filme();
        else if (opc == 5) calcular_rota_entre_filmes();
        else if (opc != 0) printf("Opção inválida. Tente novamente.\n");

    } while (opc != 0);

    return 0;
}
