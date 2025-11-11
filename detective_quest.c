#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_SIZE 101
#define MAX_INPUT 128

/* Estruturas */
typedef struct Sala {
    char *nome;
    char *pista;
    struct Sala *esq;
    struct Sala *dir;
} Sala;

typedef struct PistaNode {
    char *pista;
    int contador;
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

typedef struct HashEntry {
    char *pista;
    char *suspeito;
    struct HashEntry *prox;
} HashEntry;

/* Protótipos */
Sala* criarSala(const char *nome, const char *pista);
void explorarSalas(Sala *atual, PistaNode **rootPistas, HashEntry *hash[]);
void inserirPista(PistaNode **root, const char *pista);
void adicionarPista(PistaNode **root, const char *pista);
void inserirNaHash(HashEntry *hash[], const char *pista, const char *suspeito);
const char* encontrarSuspeito(HashEntry *hash[], const char *pista);
int verificarSuspeitoFinal(PistaNode *root, HashEntry *hash[], const char *acusado);

/* Auxiliares */
unsigned long hash_str(const char *str);
void printPistasInOrder(PistaNode *root);
void liberarPistas(PistaNode *root);
void liberarHash(HashEntry *hash[]);
char* strndup_local(const char *s);
PistaNode* criarPistaNode(const char *pista);
static int contarPistasParaSuspeito(PistaNode *root, HashEntry *hash[], const char *acusado);

/* Implementação */

char* strndup_local(const char *s) {
    if (!s) return NULL;
    size_t l = strlen(s);
    char *p = (char*) malloc(l + 1);
    if (!p) { fprintf(stderr, "Erro alocando string\n"); exit(EXIT_FAILURE); }
    memcpy(p, s, l + 1);
    return p;
}

/* criarSala - cria dinamicamente um cômodo */
Sala* criarSala(const char *nome, const char *pista) {
    Sala *s = (Sala*) malloc(sizeof(Sala));
    if (!s) { fprintf(stderr, "Erro de alocacao para Sala\n"); exit(EXIT_FAILURE); }
    s->nome = strndup_local(nome);
    s->pista = pista ? strndup_local(pista) : NULL;
    s->esq = s->dir = NULL;
    return s;
}

/* explorarSalas - navega pela árvore da mansão de forma interativa */
void explorarSalas(Sala *atual, PistaNode **rootPistas, HashEntry *hash[]) {
    Sala *cursor = atual;
    char entrada[MAX_INPUT];

    printf("\n--- Inicio da exploracao da mansao ---\n");
    while (cursor) {
        printf("\nVoce esta na sala: %s\n", cursor->nome);
        if (cursor->pista) {
            printf("Pista encontrada: %s\n", cursor->pista);
            adicionarPista(rootPistas, cursor->pista);
        } else {
            printf("Nenhuma pista visivel aqui.\n");
        }

        printf("\nEscolhas: (e) esquerda, (d) direita, (s) sair da exploracao\n");
        printf("Escolha: ");
        if (!fgets(entrada, sizeof(entrada), stdin)) break;
        char op = tolower((unsigned char)entrada[0]);
        if (op == 's') {
            printf("Voce decidiu sair da exploracao.\n");
            break;
        } else if (op == 'e') {
            if (cursor->esq) cursor = cursor->esq;
            else printf("Nao ha sala a esquerda. Escolha outra direcao.\n");
        } else if (op == 'd') {
            if (cursor->dir) cursor = cursor->dir;
            else printf("Nao ha sala a direita. Escolha outra direcao.\n");
        } else {
            printf("Entrada invalida. Use 'e', 'd' ou 's'.\n");
        }
    }
    printf("\n--- Fim da exploracao ---\n");
}

PistaNode* criarPistaNode(const char *pista) {
    PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
    if (!n) { fprintf(stderr, "Erro alocando PistaNode\n"); exit(EXIT_FAILURE); }
    n->pista = strndup_local(pista);
    n->contador = 1;
    n->esq = n->dir = NULL;
    return n;
}

/* inserirPista - insere/atualiza uma pista na BST */
void inserirPista(PistaNode **root, const char *pista) {
    if (root == NULL) return;
    if (*root == NULL) {
        *root = criarPistaNode(pista);
        return;
    }
    int cmp = strcmp(pista, (*root)->pista);
    if (cmp == 0) {
        (*root)->contador++;
    } else if (cmp < 0) {
        inserirPista(&((*root)->esq), pista);
    } else {
        inserirPista(&((*root)->dir), pista);
    }
}

/* adicionarPista - wrapper */
void adicionarPista(PistaNode **root, const char *pista) {
    inserirPista(root, pista);
    printf("[INFO] Pista '%s' adicionada/atualizada.\n", pista);
}

/* hash djb2 */
unsigned long hash_str(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

/* inserirNaHash - insere associação pista -> suspeito */
void inserirNaHash(HashEntry *hash[], const char *pista, const char *suspeito) {
    if (!pista || !suspeito) return;
    unsigned long h = hash_str(pista) % HASH_SIZE;
    HashEntry *ptr = hash[h];
    while (ptr) {
        if (strcmp(ptr->pista, pista) == 0) {
            free(ptr->suspeito);
            ptr->suspeito = strndup_local(suspeito);
            return;
        }
        ptr = ptr->prox;
    }
    HashEntry *novo = (HashEntry*) malloc(sizeof(HashEntry));
    if (!novo) { fprintf(stderr, "Erro alocando HashEntry\n"); exit(EXIT_FAILURE); }
    novo->pista = strndup_local(pista);
    novo->suspeito = strndup_local(suspeito);
    novo->prox = hash[h];
    hash[h] = novo;
}

/* encontrarSuspeito - consulta a tabela hash */
const char* encontrarSuspeito(HashEntry *hash[], const char *pista) {
    if (!pista) return NULL;
    unsigned long h = hash_str(pista) % HASH_SIZE;
    HashEntry *ptr = hash[h];
    while (ptr) {
        if (strcmp(ptr->pista, pista) == 0) return ptr->suspeito;
        ptr = ptr->prox;
    }
    return NULL;
}

/* print inorder das pistas */
void printPistasInOrder(PistaNode *root) {
    if (!root) return;
    printPistasInOrder(root->esq);
    printf(" - '%s' x%d\n", root->pista, root->contador);
    printPistasInOrder(root->dir);
}

/* contar recursivamente pistas que apontam para acusado */
static int contarPistasParaSuspeito(PistaNode *root, HashEntry *hash[], const char *acusado) {
    if (!root) return 0;
    int total = 0;
    total += contarPistasParaSuspeito(root->esq, hash, acusado);
    const char *s = encontrarSuspeito(hash, root->pista);
    if (s && strcmp(s, acusado) == 0) total += root->contador;
    total += contarPistasParaSuspeito(root->dir, hash, acusado);
    return total;
}

/* verificarSuspeitoFinal - verifica se >= 2 pistas apontam ao acusado */
int verificarSuspeitoFinal(PistaNode *root, HashEntry *hash[], const char *acusado) {
    if (!root || !acusado) return 0;
    int cont = contarPistasParaSuspeito(root, hash, acusado);
    return (cont >= 2) ? 1 : 0;
}

/* limpeza */
void liberarPistas(PistaNode *root) {
    if (!root) return;
    liberarPistas(root->esq);
    liberarPistas(root->dir);
    free(root->pista);
    free(root);
}

void liberarHash(HashEntry *hash[]) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashEntry *ptr = hash[i];
        while (ptr) {
            HashEntry *tmp = ptr;
            ptr = ptr->prox;
            free(tmp->pista);
            free(tmp->suspeito);
            free(tmp);
        }
        hash[i] = NULL;
    }
}

/* main - monta mansao, hash, explora e faz acusacao */
int main() {
    HashEntry *hash[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; i++) hash[i] = NULL;

    Sala *hall = criarSala("Hall", "pegadas molhadas");
    Sala *cozinha = criarSala("Cozinha", "faca com sangue");
    Sala *sala = criarSala("Sala de Estar", "vidro quebrado");
    Sala *porao = criarSala("Porao", "corda amarrada");
    Sala *despensa = criarSala("Despensa", "luvas de couro");
    Sala *biblioteca = criarSala("Biblioteca", "bilhete rasgado");
    Sala *quarto = criarSala("Quarto", "cartao de hotel");

    hall->esq = cozinha;
    hall->dir = sala;
    cozinha->esq = porao;
    cozinha->dir = despensa;
    sala->esq = biblioteca;
    sala->dir = quarto;

    inserirNaHash(hash, "pegadas molhadas", "Srta. Verde");
    inserirNaHash(hash, "faca com sangue", "Chef Alonzo");
    inserirNaHash(hash, "vidro quebrado", "Sr. Branco");
    inserirNaHash(hash, "corda amarrada", "Srta. Verde");
    inserirNaHash(hash, "luvas de couro", "Sr. Preto");
    inserirNaHash(hash, "bilhete rasgado", "Sr. Branco");
    inserirNaHash(hash, "cartao de hotel", "Sr. Preto");

    PistaNode *rootPistas = NULL;

    printf("=== Detective Quest (Prova de Logica) ===\n");
    printf("Explore a mansao e colete pistas. Navegue com: 'e' = esquerda, 'd' = direita, 's' = sair.\n");

    explorarSalas(hall, &rootPistas, hash);

    printf("\nPistas coletadas (ordenadas):\n");
    if (rootPistas == NULL) {
        printf("Nenhuma pista foi coletada.\n");
    } else {
        printPistasInOrder(rootPistas);
    }

    char acusado[MAX_INPUT];
    printf("\nQuem voce acusa? Digite o nome completo do suspeito (ex: 'Srta. Verde'): ");
    if (!fgets(acusado, sizeof(acusado), stdin)) {
        printf("Entrada invalida. Encerrando.\n");
    } else {
        acusado[strcspn(acusado, "\n")] = '\0';
        if (strlen(acusado) == 0) {
            printf("Nenhum nome fornecido. Encerrando.\n");
        } else {
            int ok = verificarSuspeitoFinal(rootPistas, hash, acusado);
            if (ok) {
                printf("\nResultado: Boa investigacao! Ha pistas suficientes para sustentar a acusacao contra '%s'.\n", acusado);
            } else {
                printf("\nResultado: Nao ha pistas suficientes para acusar '%s'. Reuna mais evidencias.\n", acusado);
            }
        }
    }

    liberarPistas(rootPistas);
    liberarHash(hash);

    /* libera salas */
    free(hall->nome); free(hall->pista); free(hall);
    free(cozinha->nome); free(cozinha->pista); free(cozinha);
    free(sala->nome); free(sala->pista); free(sala);
    free(porao->nome); free(porao->pista); free(porao);
    free(despensa->nome); free(despensa->pista); free(despensa);
    free(biblioteca->nome); free(biblioteca->pista); free(biblioteca);
    free(quarto->nome); free(quarto->pista); free(quarto);

    printf("\nObrigado por jogar Detective Quest!\n");
    return 0;
}
