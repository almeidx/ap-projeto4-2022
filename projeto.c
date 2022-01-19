#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_LENGTH 150
#define TAMANHO_NOME 50
#define NÚMERO_REFEIÇÕES 10
#define MAX_ALIMENTOS 10
#define N_CAMPOS_ALIMENTOS 6

#define FICHEIRO_ALIMENTOS "Alimentos.txt"
#define FICHEIRO_PESSOAS_TXT "Pessoas.txt"
#define FICHEIRO_PESSOAS_DAT "Pessoas.dat"

typedef enum Boolean { false, true } bool;
typedef char LinhaTexto[STRING_LENGTH];

LinhaTexto LT;

void clearTerminal() {
  // Provavelmente só funciona em sistemas Windows :/
  system("cls");
}

// Usar #pragma para definir regiões para ser mais fácil navegar o ficheiro
// (pode não ser suportado em todos os IDEs)
// https://stackoverflow.com/a/9000516/11252146
#pragma region Structs

typedef struct Data {
  unsigned int ano, mes, dia;
} DATA;

typedef struct IndiceColesterol {
  int total, hdl, ldl;
} INDICE_COLESTEROL;

typedef struct Pessoa {
  unsigned int cc;
  char nome[TAMANHO_NOME], morada[STRING_LENGTH], localidade[STRING_LENGTH];
  DATA dataNascimento;
  unsigned int códigoPostal, telefone, peso, altura, presaoArterial;
  INDICE_COLESTEROL indiceColesterol;
} PESSOA;

typedef struct Alimento {
  char nome[TAMANHO_NOME];
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

#pragma endregion

#pragma region Files

// Esta função está disponível no Moodle
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

void readFileContent(FILE *f, char **str, long *length) {
  // Mover o ponteiro para o fim do ficheiro
  fseek(f, 0, SEEK_END);
  *length = ftell(f);
  // Voltar para o início
  fseek(f, 0, SEEK_SET);

  // Alocar memória necessária consoante o tamanho do ficheiro
  *str = calloc(*length, sizeof(char **));
  if (*str == NULL) {
    printf("Fatal: Failed to allocate memory for file content\n");
    exit(EXIT_FAILURE);
  }

  // Ler o conteudo do ficheiro para a memória alocada
  fread(*str, sizeof(char **), *length, f);
}

FILE *openFile(char *fileName, char *mode) {
  FILE *f = fopen(fileName, mode);

  if (f == NULL) {
    printf("Fatal: erro ao abrir o ficheiro %s\n", fileName);
    exit(EXIT_FAILURE);
  }

  return f;
}

#pragma endregion Files

#pragma region Alimentos

void consultarAlimentos() {
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "r");

  char nome[TAMANHO_NOME];

  printf("Qual é o nome do alimento que deseja consultar?\n");
  scanf("%s", nome);

  int camposLidos;
  bool found = false;

  while (!feof(f)) {
    char **line = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");

    for (int i = 0; i < camposLidos; i++) {
      if (i == 0 && strcmp(line[i], nome) == 0) {
        clearTerminal();

        printf("Nome: %s\n", line[i]);
        printf("Calorias (KCal): %s\n", line[i + 1]);
        printf("Proteínas (g): %s\n", line[i + 2]);
        printf("Gorduras (g): %s\n", line[i + 3]);
        printf("Hidratos de Carbono (g): %s\n", line[i + 4]);
        printf("Grupo: %s\n", line[i + 5]);

        found = true;
        break;
      }
    }

    for (int i = 0; i < camposLidos; i++) free(line[i]);

    free(line);

    if (found) break;
  }

  if (!found) {
    printf("Alimento '%s' não encontrado.\n", nome);
  }

  fclose(f);
}

void inserirAlimento() {
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "a");

  char nome[TAMANHO_NOME];
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
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "r");

  char nome[TAMANHO_NOME];

  printf("Qual é o nome do alimento que deseja alterar?\n");
  scanf("%s", nome);

  int nLinhasLidas = 0, nCamposLidos;
  bool found = false;

  char **info;

  while (!feof(f)) {
    info = splitLine(f, N_CAMPOS_ALIMENTOS, &nCamposLidos, ";");

    nLinhasLidas++;

    for (int i = 0; i < nCamposLidos; i++) {
      if (i == 0 && strcmp(info[i], nome) == 0) {
        printf("Found element with name %s\n", nome);
        found = true;
      }
    }

    if (found) {
      break;
    }

    for (int i = 0; i < nCamposLidos; i++) free(info[i]);

    free(info);
  }

  if (!found) {
    printf("Alimento não encontrado.\n");
    return;
  }

  int option;
  do {
    printf(
        "O que deseja alterar?\n"
        "1 - nome\n"
        "2 - calorias\n"
        "3 - protainas\n"
        "4 - gorduras\n"
        "5 - hidratos de carbono\n"
        "6 - grupo\n"
        "0 - voltar\n");

    scanf("%d", &option);
  } while (option < 0 || option > 6);

  // long pos = ftell(f);

  char *fileContent;
  long fileLength;

  readFileContent(f, &fileContent, &fileLength);

  fclose(f);

  f = openFile(FICHEIRO_ALIMENTOS, "w");

  fwrite(fileContent, sizeof(*fileContent), fileLength, f);

  // ALIMENTOS;CALORIAS(KCal);PROTEINAS(g);GORDURAS(g);H.CARBONO(g);Grupo

  // switch (option) {
  //   case 1: {  // Nome
  //     char novoNome[STRING_LENGTH];

  //     printf("Qual é o novo nome do alimento?\n");
  //     scanf("%s", novoNome);

  //     fprintf(f, "%s;%s;%s;%s;%s;%s\n", novoNome, info[1], info[2],
  //     info[3],
  //             info[4], info[5]);

  //     break;
  //   }

  //   case 2: {  // Calorias
  //     int calorias;

  //     printf("Qual é a nova quantidade de calorias do alimento
  //     (KCal)?\n"); scanf("%d", &calorias);

  //     fprintf(f, "%s;%d;%s;%s;%s;%s\n", info[0], calorias, info[2],
  //     info[3],
  //             info[4], info[5]);

  //     break;
  //   }

  //   case 3: {  // Proteínas
  //     float proteínas;

  //     printf("Qual é a nova quantidade de proteínas do alimento (g)?\n");
  //     scanf("%.2f", &proteínas);

  //     fprintf(f, "%s;%s;%f;%s;%s;%s\n", info[0], info[1], proteínas,
  //     info[3],
  //             info[4], info[5]);

  //     break;
  //   }

  //   case 4: {  // Gorduras
  //     float gorduras;

  //     printf("Qual é a nova quantidade de gorduras do alimento (g)?\n");
  //     scanf("%.2f", &gorduras);

  //     fprintf(f, "%s;%s;%s;%f;%s;%s\n", info[0], info[1], info[2],
  //     gorduras,
  //             info[4], info[5]);

  //     break;
  //   }

  //   case 5: {  // Hidratos de Carbono
  //     float hidratoCarbono;

  //     printf(
  //         "Qual é a nova quantidade de hidratos de carbono do alimento "
  //         "(g)?\n");
  //     scanf("%.2f", &hidratoCarbono);

  //     fprintf(f, "%s;%s;%s;%s;%f;%s\n", info[0], info[1], info[2],
  //     info[3],
  //             hidratoCarbono, info[5]);

  //     break;
  //   }

  //   case 6: {  // Grupo
  //     int grupo;

  //     printf("Qual é o novo grupo do alimento (nº)?\n");
  //     scanf("%d", &grupo);

  //     fprintf(f, "%s;%s;%s;%s;%s;%d\n", info[0], info[1], info[2],
  //     info[3],
  //             info[4], grupo);

  //     break;
  //   }

  //   default:
  //     break;
  // }

  free(info);
  free(fileContent);

  fclose(f);
}

#pragma endregion Alimentos

#pragma region InteractiveMenu

// Isto é necessário porque as duas funções chamam uma à outra em certas
// condições
void startInteractiveMenu();

void startAlimentosInteractiveMenu() {
  short option;
  do {
    clearTerminal();

    printf(
        "O que deseja fazer aos alimentos?\n"
        " 1 - consultar\n"
        " 2 - inserir\n"
        " 3 - alterar\n"
        " 4 - eliminar\n"
        " 0 - voltar\n");
    scanf("%d", &option);
  } while (option > 4 || option < 0);

  clearTerminal();

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
      // eliminarAlimento();
      break;
    case 0:  // voltar
      startInteractiveMenu();
      return;
  }
}

void startInteractiveMenu() {
  short option;
  do {
    clearTerminal();

    printf(
        "O que deseja fazer?\n"
        " 1 - alimentos\n"
        " 2 - utentes\n"
        " 3 - gerar refeições\n"
        " 0 - sair\n");
    scanf("%d", &option);
  } while (option > 3 || option < 0);

  switch (option) {
    case 1:  // alimentos
      startAlimentosInteractiveMenu();
      break;

    case 2:  // utentes
      // char tipoFicheiro[5];
      // do {
      //   printf("Qual é o tipo de ficheiro que deseja usar? (txt ou dat) ");
      //   scanf("%s", tipoFicheiro);
      // } while (strcmp(tipoFicheiro, "txt") && strcmp(tipoFicheiro, "dat"));
      break;

    case 3:  // gerar refeições
      break;

    case 0:
      return;
  }
}

#pragma endregion InteractiveMenu

void main() {
  setlocale(LC_ALL, "Portuguese");

  startInteractiveMenu();
}
