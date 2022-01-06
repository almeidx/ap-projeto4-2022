#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_LENGTH 150
#define NÚMERO_REFEIÇÕES 10
#define MAX_ALIMENTOS 10
#define CAMPOS_ALIMENTOS 6
#define STRING char *

typedef enum { false, true } bool;
typedef char LinhaTexto[STRING_LENGTH];

LinhaTexto LT;

typedef struct Data {
  unsigned int ano, mes, dia;
} DATA;

typedef struct IndiceColesterol {
  int total, hdl, ldl;
} INDICE_COLESTEROL;

typedef struct Pessoa {
  unsigned int cc;
  char nome[STRING_LENGTH], morada[STRING_LENGTH], localidade[STRING_LENGTH];
  DATA dataNascimento;
  unsigned int códigoPostal, telefone, peso, altura, presaoArterial;
  INDICE_COLESTEROL indiceColesterol;
} PESSOA;

typedef struct Alimento {
  char nome[STRING_LENGTH];
  unsigned int quantidade;
} ALIMENTO;

typedef struct Refeição {
  ALIMENTO alimentos[MAX_ALIMENTOS];
} REFEIÇÃO;

typedef struct Menu {
  unsigned int cc;
  DATA dataRefeição;
  // 1-Pequeno Almoço, A-Almoço, 3-Lanche, 4-Jantar, 5-Ceia, 6-Outra
  unsigned short refeiçãoDoDia;
  float kCalorias, minProteínas, maxProteínas, gorduras, hidratosCarbono;
  REFEIÇÃO refeição[NÚMERO_REFEIÇÕES];
} MENU;

/**
  Nome-da-função: Read_Split_Line_File
  Descrição: Permite ler o conteúdo de uma linha do ficheiro e devolve
            uma tabela de strings com os vários campos lidos;
  Parametros:
  - F :	ficheiro onde vai ser feita a leitura
  - n_campos_max:	n�mero de campos máximo a ler
  - n_campos_lidos:	devolve o numero de campos que a linha cont�m
  - separadores :	permite definir os delimitadores dos campos que estão na
  linha do ficheiro Retorno:
  - Retorna uma tabela de strings
  Observação:
  - Dentro desta função é alocado espaço de memória, ficando a cargo
    de quem invoca a função a libertação da memória alocada
  Autor: Francisco Morgado
**/
STRING *Read_Split_Line_File(FILE *f, int n_campos_max, int *n_campos_lidos,
                             char *separadores) {
  *n_campos_lidos = 0;
  if (!f) return NULL;
  if (fgets(LT, STRING_LENGTH, f) !=
      NULL)  // fgets lê uma linha do ficheiro de texto para a string LT
  {
    // "partir" a linha lida, usando os separadores definidos
    STRING *Res = (STRING *)malloc(
        n_campos_max * sizeof(STRING));  // alocação de um array com
                                         // n_campos_max ponteiros para STRING
    char *pch = strtok(LT, separadores);
    int cont = 0;
    while (pch != NULL) {
      Res[cont] = (STRING)malloc(
          (strlen(pch) + 1) *
          sizeof(char));  // alocação do espaço necessário para guardar a string
                          // correspondente ao campo
      strcpy(Res[cont++], pch);
      pch = strtok(NULL, separadores);
    }
    *n_campos_lidos = cont;
    return Res;
  }

  return NULL;
};

void openFile(FILE *f, STRING fileName, STRING mode) {
  f = fopen(fileName, mode);

  if (f == NULL) {
    printf("Erro ao abrir o ficheiro %s.\n", fileName);
    exit(EXIT_FAILURE);
  }
}

void consultarAlimentos() {
  FILE *f = fopen("Alimentos.txt", "r");

  if (f == NULL) {
    printf("Erro ao abrir o ficheiro Alimentos.txt.\n");
    exit(EXIT_FAILURE);
  }

  char nome[STRING_LENGTH];

  printf("Qual é o nome do alimento que deseja consultar?\n");
  scanf("%s", nome);

  int nCamposLidos;
  bool found = false;

  while (!feof(f)) {
    char **V = Read_Split_Line_File(f, CAMPOS_ALIMENTOS, &nCamposLidos, ";");

    for (int i = 0; i < nCamposLidos; i++) {
      if (i == 0 && strcmp(V[i], nome) == 0) {
        printf("Nome: %s\n", V[i]);
        printf("Calorias (KCal): %s\n", V[i + 1]);
        printf("Proteínas (g): %s\n", V[i + 2]);
        printf("Gorduras (g): %s\n", V[i + 3]);
        printf("Hidratos de Carbono (g): %s\n", V[i + 4]);
        printf("Grupo: %s\n", V[i + 5]);

        found = true;
        break;
      }
    }

    for (int i = 0; i < nCamposLidos; i++) free(V[i]);

    free(V);
  }

  if (!found) {
    printf("Alimento não encontrado.\n");
  }

  fclose(f);
}

void inserirAlimento() {
  FILE *f = fopen("Alimentos.txt", "a");

  if (f == NULL) {
    printf("Erro ao abrir o ficheiro Alimentos.txt.\n");
    exit(EXIT_FAILURE);
  }

  char nome[STRING_LENGTH];
  int grupo;
  float calorias, proteína, gordura, hidratocarbono;

  printf("Qual é o nome do alimento que deseja inserir?\n");
  scanf("%s", nome);

  printf("Qual é o grupo do alimento que deseja inserir?\n");
  scanf("%s", grupo);

  printf("Qual é a quantidade de calorias do alimento que deseja inserir?\n");
  scanf("%f", &calorias);

  printf("Qual é a quantidade de proteínas do alimento que deseja inserir?\n");
  scanf("%f", &proteína);

  printf("Qual é a quantidade de gorduras do alimento que deseja inserir?\n");
  scanf("%f", &gordura);

  printf(
      "Qual é a quantidade de hidratos de carbono do alimento que "
      "deseja inserir?\n");
  scanf("%f", &hidratocarbono);

  fprintf(f, "%s;%d;%d;%d;%d;%s\n", nome, calorias, proteína, gordura,
          hidratocarbono, grupo);

  fclose(f);
}

void main() {
  setlocale(LC_ALL, "Portuguese");

  //#region Ler ficheiro

  // char tipoFicheiro[5];
  // do {
  //   printf("Qual é o tipo de ficheiro que deseja usar? (txt ou dat) ");
  //   scanf("%s", tipoFicheiro);
  // } while (strcmp(tipoFicheiro, "txt") && strcmp(tipoFicheiro, "dat"));

  // FILE *ficheiroPessoas;
  // if (!strcmp(tipoFicheiro, "txt")) {
  //   openFile(ficheiroPessoas, "Pessoas.txt", "r+");
  // } else {
  //   openFile(ficheiroPessoas, "Pessoas.dat", "rb+");
  // }

  // if (ficheiroPessoas == NULL) {
  //   printf("Ocorreu um erro ao abrir o ficheiro das pessoas!\n");
  //   exit(1);
  // }

  //#endregion

  //#region Operações com alimentos

  char op;
  do {
    printf(
        "O que deseja fazer (alimentos)? (c-consultar, i-inserir, a-alterar, "
        "e-eliminar, "
        "s-sair) ");
    scanf("%c", &op);
  } while (op != 'c' && op != 'i' && op != 'a' && op != 'e' && op != 's');

  switch (op) {
    case 'c':  // consultar
      consultarAlimentos();
      break;
    case 'i':  // inserir
      break;
    case 'a':  // alterar
      break;
    case 'e':  // eliminar
      break;
    default:  // sair
      exit(0);
      break;
  }

  //#endregion
}
