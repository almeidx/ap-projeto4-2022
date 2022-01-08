#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_LENGTH 150
#define NÚMERO_REFEIÇÕES 10
#define MAX_ALIMENTOS 10
#define CAMPOS_ALIMENTOS 6

#define FICHEIRO_PESSOAS_TXT "Pessoas.txt"
#define FICHEIRO_PESSOAS_DAT "Pessoas.dat"

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
char **splitLine(FILE *f, int n_campos_max, int *n_campos_lidos,
                 char *separadores) {
  *n_campos_lidos = 0;
  if (!f) return NULL;

  if (fgets(LT, STRING_LENGTH, f) !=
      NULL)  // fgets lê uma linha do ficheiro de texto para a string LT
  {
    // "partir" a linha lida, usando os separadores definidos
    char **Res = (char **)malloc(
        n_campos_max * sizeof(char *));  // alocação de um array com
                                         // n_campos_max ponteiros para STRING
    char *pch = strtok(LT, separadores);
    int cont = 0;
    while (pch != NULL) {
      Res[cont] = (char *)malloc(
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

void clearBuffer() { system("cls"); }

FILE *openFile(char *filename, char *mode) {
  FILE *f = fopen(filename, mode);

  if (f == NULL) {
    printf("Erro ao abrir o ficheiro %s\n", filename);
    exit(EXIT_FAILURE);
  }

  return f;
}

void consultarAlimentos() {
  FILE *f = openFile("Alimentos.txt", "r");

  char nome[STRING_LENGTH];

  printf("Qual é o nome do alimento que deseja consultar?\n");
  scanf("%s", nome);

  int nCamposLidos;
  bool found = false;

  while (!feof(f)) {
    char **V = splitLine(f, CAMPOS_ALIMENTOS, &nCamposLidos, ";");

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
  FILE *f = openFile("Alimentos.txt", "a+");

  char nome[STRING_LENGTH];
  int grupo;
  float calorias, proteína, gordura, hidratocarbono;

  printf("Qual é o nome do alimento?\n");
  scanf("%s", nome);

  printf("Qual é o grupo do alimento?\n");
  scanf("%s", &grupo);

  printf("Qual é a quantidade de calorias do alimento?\n");
  scanf("%f", &calorias);

  printf("Qual é a quantidade de proteínas do alimento?\n");
  scanf("%f", &proteína);

  printf("Qual é a quantidade de gorduras do alimento?\n");
  scanf("%f", &gordura);

  printf("Qual é a quantidade de hidratos de carbono do alimento?\n");
  scanf("%f", &hidratocarbono);

  fprintf(f, "%s;%d;%d;%d;%d;%s\n", nome, calorias, proteína, gordura,
          hidratocarbono, grupo);

  fclose(f);
}

void alterarAlimento() {
  FILE *f = openFile("Alimentos.txt", "r+");

  char nome[STRING_LENGTH];

  printf("Qual é o nome do alimento que deseja alterar?\n");
  scanf("%s", nome);

  int nCamposLidos, nLinhasLidas = 0;
  bool found = false;

  char *info = (char *)malloc(STRING_LENGTH * sizeof(char));

  while (!feof(f)) {
    char **V = splitLine(f, CAMPOS_ALIMENTOS, &nCamposLidos, ";");

    nLinhasLidas++;

    for (int i = 0; i < nCamposLidos; i++) {
      if (i == 0 && strcmp(V[i], nome) == 0) {
        clearBuffer();

        info = V;

        found = true;
        break;
      }
    }

    for (int i = 0; i < nCamposLidos; i++) free(V[i]);

    free(V);

    if (found) {
      break;
    }
  }

  if (!found) {
    printf("Alimento não encontrado.\n");
  }

  int option;
  do {
    printf(
        "O que deseja alterar?\n"
        "1 - Nome\n"
        "2 - Calorias\n"
        "3 - Protainas\n"
        "4 - Gorduras\n"
        "5 - Hidratos de Carbono\n"
        "6 - Cancelar");

    scanf("%d", &option);
  } while (option < 1 || option > 6);

  switch (option) {
    case 1: {
      // Nome
      printf("Qual é o novo nome do alimento?\n");
      scanf("%s", nome);

      fseek(f, -(nCamposLidos * sizeof(char *)), SEEK_CUR);
      fprintf(f, "%s", nome);
      break;
    }
  }

  fclose(f);
}

void pickOptionAlimentos() {
  int option;

  do {
    clearBuffer();

    printf(
        "O que deseja fazer aos alimentos?\n"
        " 1 - consultar\n"
        " 2 - inserir\n"
        " 3 - alterar\n"
        " 4 - eliminar\n"
        " 5 - sair\n");
    scanf("%d", &option);
  } while (option > 5 || option < 1);

  clearBuffer();

  switch (option) {
    case 1:
      consultarAlimentos();
      break;
    case 2:
      inserirAlimento();
      break;
    case 3:
      alterarAlimento();
      break;
    case 4:
      break;
    case 5:
      exit(EXIT_SUCCESS);
  }
}

void main() {
  setlocale(LC_ALL, "Portuguese");

  int option;
  do {
    clearBuffer();

    printf(
        "O que deseja fazer?\n"
        " 1 - alimentos\n"
        " 2 - utentes\n"
        " 3 - gerar refeições\n"
        " 4 - sair\n");
    scanf("%d", &option);
  } while (option > 4 || option < 1);

  switch (option) {
    case 1: {  // alimentos
      pickOptionAlimentos();
      break;
    }

    case 2: {  // utentes
      char tipoFicheiro[5];
      do {
        printf("Qual é o tipo de ficheiro que deseja usar? (txt ou dat) ");
        scanf("%s", tipoFicheiro);
      } while (strcmp(tipoFicheiro, "txt") && strcmp(tipoFicheiro, "dat"));

      break;
    }

    case 3: {  // gerar refeições
      break;
    }

    case 4: {
      exit(EXIT_SUCCESS);
    }
  }
}
