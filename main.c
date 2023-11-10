#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

#define MAX 1024

void menu(int *status, int id);
int nova_venda(sqlite3 *db, int *status);
int adicionar_carrinho(sqlite3 *db, int id);
int fechar_venda(sqlite3 *db, int id);
int gerenciar_estoque(sqlite3 *db);

void exibe_estoque(sqlite3 *db);
void exibe_venda(sqlite3 *db, int id);
void lerString(char s[]);

int main()
{
    int status = 0;
    int *status_venda = &status;
    int id_vendas = -1;
    menu(status_venda, id_vendas);
    return 0;
}

void menu(int *status, int id) {
    sqlite3 *db = NULL;
    int conexao;
    conexao = sqlite3_open("lojinha.db", &db);
    if (conexao != SQLITE_OK){
    exit(-1);
    }
    int op;
    printf("1 - Nova venda\n");
    printf("2 - Adicionar ao carrinho\n");
    printf("3 - Fechar venda\n");
    printf("4 - Gerenciar estoque\n");
    printf("5 - Sair\n");
    printf("Escolha: ");
    scanf("%i", &op);
    switch (op){
    case 1:
        id = nova_venda(db, status);
        menu(status, id);
        break;
    case 2:
        if (*status == 0) {
            system("cls");
            printf("Nenhuma venda aberta!\n");
            menu(status, id);
        }
        adicionar_carrinho(db, id);
        menu(status, id);
        break;
    case 3:
        if (*status == 0) {
            system("cls");
            printf("Nenhuma venda aberta!\n");
            menu(status, id);
        }
        exibe_venda(db, id);
        fechar_venda(db, id);
        *status = 0;
        break;
    case 4:
        gerenciar_estoque(db);
        menu(status, id);
        break;
    case 5:
        sqlite3_close(db);
        exit(-1);
        break;
    default:
        system("cls");
        printf("Opcao invalida!\n");
        menu(status, id);
    }
}

int nova_venda(sqlite3 *db, int *status) {
    char sql[MAX];
    sqlite3_stmt *stmt = NULL;
    int id_vendas;
    sprintf(sql, "SELECT MAX(idVenda) FROM vendas");
    sqlite3_prepare(db, sql, -1, &stmt, NULL);
    if (sqlite3_column_int(stmt, 0) == SQLITE_DONE) {
        id_vendas = 1;
    }
    if (*status == 0) {
        *status = 1;
        id_vendas = sqlite3_column_int(stmt, 0) + 1;
    }
    else {
        id_vendas = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    system("cls");
    return id_vendas;
}

int adicionar_carrinho(sqlite3 *db, int id) {
    int codigo, qtd;
    float preco;
    char sql[MAX];
    int i = 0;
    sqlite3_stmt *stmt = NULL;

    exibe_estoque(db);
    sqlite3_finalize(stmt);
    printf("Digite o codigo seguido da quantidade (codigo-qtd): ");
    scanf("%i %i", &codigo, &qtd);
    sprintf(sql, "SELECT preco, quant FROM estoque WHERE (codigo = %i)", codigo);
    sqlite3_prepare(db, sql, -1, &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("O produto não existe!\n");
        return -1;
    }
    if (sqlite3_column_int(stmt, 1) < qtd) {
        printf("A quantidade informada e superior a disponivel no estoque!\n");
        return -2;
    }
    preco = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);

    sprintf(sql, "SELECT codigo FROM vendas");
    sqlite3_prepare(db, sql, -1, &stmt, NULL);

    while (sqlite3_step(stmt) != SQLITE_DONE) {
        if (sqlite3_column_double(stmt, 0) == codigo) {
            sqlite3_finalize(stmt);
            sprintf(sql, "UPDATE vendas SET quant = quant+%i WHERE codigo = %i", qtd, codigo);
            sqlite3_prepare(db, sql, -1, &stmt, NULL);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            i++;
            break;
        }
    }
    sqlite3_finalize(stmt);
    if (i == 0){
            sprintf(sql, "INSERT INTO vendas (idVenda, codigo, quant, preco) VALUES (%i, %i, %i, %f)", id, codigo, qtd, preco);
            sqlite3_prepare(db, sql, -1, &stmt, NULL);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
    }


    system("cls");

    exibe_venda(db, id);
    system("pause");
    system("cls");
    sprintf(sql, "UPDATE estoque SET quant=quant-%i WHERE codigo=%i", qtd, codigo);
    sqlite3_prepare(db, sql, -1, &stmt, NULL);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    printf("Estoque atualizado:\n");
    exibe_estoque(db);
    system("pause");
}

int fechar_venda(sqlite3 *db, int id) {
    float preco_total;
    char sql[MAX];
    sqlite3_stmt *stmt = NULL;
    sprintf(sql, "SELECT SUM(preco*quant) FROM vendas WHERE (idVenda = %i)", id);
    sqlite3_prepare(db, sql, -1, &stmt, NULL);
    sqlite3_step(stmt);
    preco_total = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    printf("Total da venda = %.2f\n", preco_total);
    system("pause");
    system("cls");
}

int gerenciar_estoque(sqlite3 *db) {
    int codigo, qtd;
    float preco;
    char nome[MAX];
    exibe_estoque(db);
    printf("Digite um codigo: ");
    scanf("%i", &codigo);

    char sql[MAX];
    sqlite3_stmt *stmt = NULL;
    sprintf(sql, "SELECT * FROM estoque WHERE codigo = %i", codigo);
    sqlite3_prepare(db, sql, -1, &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        printf("Digite o nome do produto a ser adicionado: ");
        lerString(nome);
        printf("Digite a quantidade do produto: ");
        scanf("%i", &qtd);
        printf("Digite o preco do produto: ");
        scanf("%f", &preco);
        sprintf(sql, "INSERT INTO estoque (codigo, nome, quant, preco) VALUES ((SELECT MAX(codigo) FROM estoque) + 1 , '%s', %i, %f)", nome, qtd, preco);
        sqlite3_prepare(db, sql, -1, &stmt, NULL);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    else {
        sqlite3_finalize(stmt);
        printf("Informe a quantidade a ser adicionada: ");
        scanf("%i", &qtd);
        printf("Informe o novo preco: ");
        scanf("%f", &preco);
        sprintf(sql, "UPDATE estoque SET quant=quant+ %i, preco = %f WHERE codigo=%i", qtd, preco, codigo);
        sqlite3_prepare(db, sql, -1, &stmt, NULL);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    system("cls");
    exibe_estoque(db);
    system("pause");
    system("cls");
}

void exibe_estoque(sqlite3 *db) {
    char sql[MAX];
    sqlite3_stmt *stmt = NULL;
    sprintf(sql, "SELECT * FROM estoque");
    sqlite3_prepare(db, sql, -1, &stmt, NULL);
    for (int i = 0; i < sqlite3_column_count(stmt); i++)
        printf("%s\t", sqlite3_column_name(stmt, i));
    printf("\n"); // termina a exibição do cabeçalho
    while (sqlite3_step(stmt) != SQLITE_DONE){
    // para cada coluna
    for (int i = 0; i < sqlite3_column_count(stmt); i++)
        printf("%s\t", sqlite3_column_text(stmt, i));
    printf("\n"); // termina a exibição de uma linha
    }
    sqlite3_finalize(stmt);
}

void exibe_venda(sqlite3 *db, int id) {
    char sql[MAX];
    sqlite3_stmt *stmt = NULL;
    sprintf(sql, "SELECT * FROM vendas WHERE (idVenda = %i)", id);
    sqlite3_prepare(db, sql, -1, &stmt, NULL);
    for (int i = 0; i < sqlite3_column_count(stmt); i++)
        printf("%s\t", sqlite3_column_name(stmt, i));
    printf("\n"); // termina a exibição do cabeçalho
    while (sqlite3_step(stmt) != SQLITE_DONE){
    // para cada coluna
    for (int i = 0; i < sqlite3_column_count(stmt); i++)
        printf("%s\t", sqlite3_column_text(stmt, i));
    printf("\n"); // termina a exibição de uma linha
    }
    sqlite3_finalize(stmt);
}

void lerString(char s[]){
setbuf(stdin,0);
fgets(s, MAX, stdin);
if(s[strlen(s) - 1] == '\n'){
s[strlen(s) - 1] = '\0';
}
}
