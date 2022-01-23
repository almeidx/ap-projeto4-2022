#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Projeto 4
 *
 * [REDACTED]
 *
 * Este projeto foi desenvolvido e testado num sistema com:
 *  - Windows 11 Pro (Versão 21H2; Build 22000.466)
 *
 * Compilado usando gcc (versão 10.3.0)
 */

#define STRING_LENGTH 150
#define TAMANHO_NOME 50

#define MAX_ALIMENTOS 10
#define N_REFEICOES 10

#define N_CAMPOS_ALIMENTOS 6
#define N_CAMPOS_UTENTES 15

#define FICHEIRO_ALIMENTOS "Alimentos.txt"
#define FICHEIRO_UTENTES_TXT "Pessoas.txt"
#define FICHEIRO_UTENTES_DAT "Pessoas.dat"
#define FICHEIRO_MENUS_DAT "Menus.dat"
#define FICHEIRO_TMP_TXT "tmp.txt"
#define FICHEIRO_TMP_DAT "tmp.dat"

typedef enum Boolean { false, true } bool;
typedef char LinhaTexto[STRING_LENGTH];

LinhaTexto LT;
bool useTextFileForUtentes = false;

void clearTerminal() {
  system("cls");
}

// Usamos #pragma para definir regiões para ser mais fácil navegar o ficheiro (pode não ser suportado em todos os IDEs)
// https://stackoverflow.com/a/9000516/11252146
#pragma region Funções para converter float para string

// A função ftoa() foi ligeiramente modificada do link abaixo, para não ter um argumento de 'afterpoint', e para não
// depender em outras 2 funções. Em vez disso, fizemos uso de itoa():
// https://www.geeksforgeeks.org/convert-floating-point-number-string/
// https://stackoverflow.com/a/8257754/11252146

// Converts a floating-point/double number to a string.
void ftoa(float n, char *str) {
  // Extract integer part
  int ipart = (int)n;

  // Extract floating part
  float fpart = n - (float)ipart;

  // convert integer part to string
  itoa(ipart, str, 10);

  int len = strlen(str);
  str[len] = '.';  // add dot

  // Get the value of fraction part upto 5 points after dot. Use to handle cases like 233.007
  fpart *= pow(10, 5);

  char floatingPart[10];
  itoa((int)fpart, floatingPart, 10);

  strcat(str, floatingPart);
}

#pragma endregion

#pragma region Structures

typedef struct Data {
  unsigned int ano, mes, dia;
} DATA;

typedef struct IndiceColesterol {
  int total, hdl, ldl;
} INDICE_COLESTEROL;

typedef struct PressaoArterial {
  unsigned int min, max;
} PRESSAO_ARTERIAL;

typedef struct Pessoa {
  unsigned int cc;
  char nome[TAMANHO_NOME], morada[STRING_LENGTH], localidade[STRING_LENGTH];
  DATA dataNascimento;
  unsigned int codigoPostal, telefone, peso, altura;
  PRESSAO_ARTERIAL pressaoArterial;
  INDICE_COLESTEROL indiceColesterol;
} PESSOA;

typedef struct Alimento {
  char nome[TAMANHO_NOME];
  unsigned int quantidade;
} ALIMENTO;

typedef struct Refeicao {
  ALIMENTO alimentos[MAX_ALIMENTOS];
} REFEICAO;

typedef struct Menu {
  unsigned int cc;
  DATA dataRefeicao;
  // 1-Pequeno Almoço, A-Almoço, 3-Lanche, 4-Jantar, 5-Ceia, 6-Outra
  unsigned short refeicaoDoDia;
  float kCalorias, minProteinas, maxProteinas, gorduras, hidratosCarbono;
  REFEICAO refeicoes[N_REFEICOES];
} MENU;

#pragma endregion

/*
 * Função que valida se uma data é válida.
 * O ano mínimo e máximo aceitável é 1900 e 2050, respetivamente.
 * Leva em conta se o ano é bissexto ou não.
 * Retorna true se a data for valida. Se não, retorna false.
 */
bool validarData(DATA data) {
  // Verificar limites genericos
  if (data.ano < 1900 || data.ano > 2050 || data.mes < 1 || data.mes > 12 || data.dia < 1 || data.dia > 31) {
    return false;
  }

  // Verificar o dia máximo de fevereiro consoante se o ano é bissexto
  if (data.mes == 2) {
    if (data.ano % 4 == 0) {
      if (data.dia > 29) return false;
    } else {
      if (data.dia > 28) return false;
    }
  }

  // Verificar o dia máximo nos meses com 30 dias
  if ((data.mes == 4 || data.mes == 6 || data.mes == 9 || data.mes == 11) && data.dia > 30) {
    return false;
  }

  return true;
}

#pragma region Files

// Esta função está disponível no Moodle
/**
  Nome-da-função: Read_Split_Line_File
  Descrição: Permite ler o conteúdo de uma linha do ficheiro e devolve
            uma tabela de strings com os vários campos lidos;
  Parametros:
  - F :	ficheiro onde vai ser feita a leitura
  - n_campos_max:	número de campos máximo a ler
  - n_campos_lidos:	devolve o numero de campos que a linha contem
  - separadores :	permite definir os delimitadores dos campos que estão na
  linha do ficheiro Retorno:
  - Retorna uma tabela de strings
  Observação:
  - Dentro desta função é alocado espaço de memória, ficando a cargo
    de quem invoca a função a libertação da memória alocada
  Autor: Francisco Morgado
**/
char **splitLine(FILE *f, int n_campos_max, int *n_campos_lidos, char *separadores) {
  *n_campos_lidos = 0;

  if (fgets(LT, STRING_LENGTH, f) != NULL)  // fgets lê uma linha do ficheiro de texto para a string LT
  {
    // "partir" a linha lida, usando os separadores definidos
    char **Res = (char **)malloc(n_campos_max * sizeof(char *));  // alocação de um array com
                                                                  // n_campos_max ponteiros para STRING
    char *pch = strtok(LT, separadores);
    int cont = 0;
    while (pch != NULL) {
      Res[cont] = (char *)malloc((strlen(pch) + 1) * sizeof(char));  // alocação do espaço necessário para guardar a
                                                                     // string correspondente ao campo
      strcpy(Res[cont++], pch);
      pch = strtok(NULL, separadores);
    }
    *n_campos_lidos = cont;
    return Res;
  }

  return NULL;
};

FILE *openFile(char *fileName, char *mode) {
  FILE *f = fopen(fileName, mode);

  if (f == NULL) {
    printf("Fatal: erro ao abrir o ficheiro %s\n", fileName);
    exit(EXIT_FAILURE);
  }

  return f;
}

#pragma endregion

#pragma region Alimentos

/*
  Grupos de Alimentos:
  1. Fruta;
  2. Gorduras e Óleos;
  3. Lacticínios;
  4. Carne, Pescado e Ovos;
  5. Leguminosas;
  6. Cereais e derivados e Tubérculos;
  7. Hortícolas;
  8. Outros.

  Tipos de Refeições:
  1. Pequeno Almoço; (1, 3, 6)
  2. Almoco; (1, 2, 4, 5, 6, 7, 8)
  3. Lanche; (1, 3, 6)
  4. Jantar; (1, 2, 4, 5, 6, 7, 8)
  5. Ceia; (1, 3, 6)
  6. Outra. (1, 2, 3, 4, 5, 6, 7, 8)
*/

// Transforma o número do grupo (que vem em forma de char) no nome do grupo (em forma de string)
char *resolveGroupName(char grupo) {
  switch (grupo) {
    case '1':
      return "Fruta";
    case '2':
      return "Gorduras e Óleos";
    case '3':
      return "Lacticínios";
    case '4':
      return "Carne, Pescado e Ovos";
    case '5':
      return "Leguminosas";
    case '6':
      return "Cereais e derivados e Tubérculos";
    case '7':
      return "Hortícolas";
    case '8':
      return "Outros";
    default:
      return "";
  }
}

/*
 * A maneira de como esta função funciona é:
 *  - cria um ficheiro temporário e vai de linha a linha, copiando todos os alimentos do ficheiro principal para o
 * temporário
 *  - quando encontrar o alimento que é para eliminar, simplesmente não vai copiar para o ficheiro temporário
 *  - no final, damos overwrite do ficheiro normal com o conteudo do ficheiro temporário
 *
 * Caso o alimento seja encontrado, retorna true. Se não for encontrado, retorna false.
 */
bool deleteAlimento(FILE *f, char *nome) {
  FILE *tmpFile = openFile(FICHEIRO_TMP_TXT, "w+");

  // Mover o ponteiro do ficheiro para o inicio
  fseek(f, 0, SEEK_SET);

  int camposLidos;
  bool found = false, ignore = false;
  char line[STRING_LENGTH], **info;

  while (!feof(f)) {
    info = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_ALIMENTOS) {
      for (int z = 0; z < camposLidos; z++) free(info[z]);
      free(info);
      continue;
    }

    for (int i = 0; i < camposLidos; i++) {
      // Se o nome for igual ao do alimento para apagar, ignorar
      if (i == 0 && strcmp(info[i], nome) == 0) {
        ignore = true;
        found = true;
      }

      // Contatenar todas as strings que foram separadas pela função splitLine
      if (!ignore) {
        // A função strcat() estava a deixar alguns caracteres nulos no inicio
        // da linha por algum motivo
        if (i == 0) {
          strcpy(line, info[0]);
        } else {
          strcat(line, info[i]);
        }

        // Adicionar o separador
        if (i != N_CAMPOS_ALIMENTOS - 1) {
          strcat(line, ";");
        }
      }
    }

    // Se não for para ignorar, copiar o conteudo da linha para o ficheiro
    // temporário
    if (!ignore) {
      fputs(line, tmpFile);
    }

    // Fazer o 'reset' das variáveis temporárias para passar à próxima linha
    strcpy(line, "");
    ignore = false;
    for (int z = 0; z < camposLidos; z++) free(info[z]);
    free(info);
  }

  fclose(tmpFile);

  // Reescrever o ficheiro de alimentos com o conteudo do ficheiro temporário
  if (found) {
    rename(FICHEIRO_TMP_TXT, FICHEIRO_ALIMENTOS);
  } else {
    remove(FICHEIRO_TMP_TXT);
  }

  return found;
}

void consultarAlimentos() {
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "r");

  char nome[TAMANHO_NOME];

  printf("Qual é o nome do alimento que deseja consultar?\n");
  scanf("%s", nome);

  clearTerminal();

  int camposLidos;
  bool found = false;
  char **line;

  while (!feof(f)) {
    line = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_ALIMENTOS) {
      for (int z = 0; z < camposLidos; z++) free(line[z]);
      free(line);
      continue;
    }

    for (int i = 0; i < camposLidos; i++) {
      if (i == 0 && strcmp(line[i], nome) == 0) {
        printf("Nome: %s\n", line[0]);
        printf("Calorias (KCal): %s\n", line[1]);
        printf("Proteínas (g): %s\n", line[2]);
        printf("Gorduras (g): %s\n", line[3]);
        printf("Hidratos de Carbono (g): %s\n", line[4]);

        // O motivo de estar a usar line[5][0] a seguir é porque a função splitLine() inclui o \n no ultimo elemento
        // do array. E, por causa disso, o nome do grupo estava a ficar numa linha diferente
        printf("Grupo: %c (%s)\n", line[5][0], resolveGroupName(line[5][0]));

        found = true;
        break;
      }
    }

    // free the line variable manually
    for (int i = 0; i < camposLidos; i++) {
      free(line[i]);
    }
    free(line);

    if (found) break;
  }

  if (!found) {
    printf("Alimento '%s' não encontrado.\n", nome);
  }

  fclose(f);
}

void inserirAlimento() {
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "a+");

  char nome[TAMANHO_NOME];
  int grupo;
  float calorias, proteinas, gordura, hidratosCarbono;

  printf("Qual é o nome do alimento?\n");
  scanf("%s", nome);

  do {
    printf("Qual é o grupo do alimento (1-8)?\n");
    scanf("%d", &grupo);
  } while (grupo < 1 || grupo > 8);

  printf("Qual é a quantidade de calorias do alimento?\n");
  scanf("%f", &calorias);

  printf("Qual é a quantidade de proteínas do alimento?\n");
  scanf("%f", &proteinas);

  printf("Qual é a quantidade de gorduras do alimento?\n");
  scanf("%f", &gordura);

  printf("Qual é a quantidade de hidratos de carbono do alimento?\n");
  scanf("%f", &hidratosCarbono);

  fprintf(f, "%s;%f;%f;%f;%f;%d\n", nome, calorias, proteinas, gordura, hidratosCarbono, grupo);

  fclose(f);
}

void alterarAlimento() {
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "r");

  char nome[TAMANHO_NOME];

  printf("Qual é o nome do alimento que deseja alterar?\n");
  scanf("%s", nome);

  int camposLidos;
  bool found = false;
  char **info;

  while (!feof(f)) {
    info = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_ALIMENTOS) {
      for (int z = 0; z < camposLidos; z++) free(info[z]);
      free(info);
      continue;
    }

    for (int i = 0; i < camposLidos; i++) {
      if (i == 0 && strcmp(info[i], nome) == 0) {
        found = true;
        break;
      }
    }

    if (found) {
      break;
    }

    for (int z = 0; z < camposLidos; z++) free(info[z]);
    free(info);
  }

  if (!found) {
    printf("Alimento '%s' não encontrado.\n", nome);
    return;
  }

  short option;
  do {
    printf(
        "O que deseja alterar?\n"
        " 1 - nome\n"
        " 2 - calorias\n"
        " 3 - protainas\n"
        " 4 - gorduras\n"
        " 5 - hidratos de carbono\n"
        " 6 - grupo\n"
        " 0 - voltar\n");

    scanf("%hd", &option);
  } while (option < 0 || option > 6);

  // ALIMENTOS;CALORIAS(KCal);PROTEINAS(g);GORDURAS(g);H.CARBONO(g);Grupo

  char newItem[TAMANHO_NOME];

  clearTerminal();

  switch (option) {
    case 1:  // Nome
      printf("Qual é o novo nome do alimento?\n");
      scanf("%s", newItem);
      break;
    case 2:  // Calorias
      printf("Qual é a nova quantidade de calorias do alimento (KCal)?\n");
      scanf("%s", newItem);
      break;
    case 3:  // Proteínas
      printf("Qual é a nova quantidade de proteínas do alimento (g)?\n");
      scanf("%s", newItem);
      break;
    case 4:  // Gorduras
      printf("Qual é a nova quantidade de gorduras do alimento (g)?\n");
      scanf("%s", newItem);
      break;
    case 5:  // Hidratos de Carbono
      printf("Qual é a nova quantidade de hidratos de carbono do alimento (g)?\n");
      scanf("%s", newItem);
      break;
    case 6:  // Grupo
      printf("Qual é o novo grupo do alimento (nº)?\n");
      scanf("%s", newItem);
      break;
    case 0:  // Voltar
      fclose(f);
      for (int i = 0; i < N_CAMPOS_ALIMENTOS; i++) free(info[i]);
      free(info);
      return;
  }

  clearTerminal();

  // Alterar o item que utilizador introduziu
  strcpy(info[option - 1], newItem);

  // Apagar o alimento anterior
  deleteAlimento(f, nome);

  // Fechar para abrir novamente em modo 'append'
  fclose(f);

  // Abrir em modo 'append'
  f = openFile(FICHEIRO_ALIMENTOS, "a+");

  // Adicionar o alimento atualizado
  fprintf(f, "\n%s;%s;%s;%s;%s;%s", info[0], info[1], info[2], info[3], info[4], info[5]);

  for (int z = 0; z < camposLidos; z++) free(info[z]);
  free(info);
  fclose(f);
}

void eliminarAlimento() {
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "r");

  char nome[TAMANHO_NOME];

  printf("Qual é o nome do alimento que quer eliminar?\n");
  scanf("%s", nome);

  clearTerminal();

  if (deleteAlimento(f, nome)) {
    printf("O alimento '%s' foi eliminado com sucesso.\n", nome);
  } else {
    printf("Não foi possível eliminar o alimento com o nome '%s'.\n", nome);
  }
}

void listarAlimentos() {
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "r");

  int camposLidos, linhasLidas = 0;
  char **line;

  clearTerminal();

  while (!feof(f)) {
    line = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_ALIMENTOS) {
      for (int i = 0; i < camposLidos; i++) free(line[i]);
      free(line);
      continue;
    }

    linhasLidas++;

    // A primeira linha não contém um alimento, mas sim a descrição de cada um dos campos
    if (linhasLidas == 1) {
      printf("\n%s | %s | %s | %s | %s | %s", line[0], line[1], line[2], line[3], line[4], line[5]);
    } else {
      printf("\n%s | %s | %s | %s | %s | %c (%s)", line[0], line[1], line[2], line[3], line[4],
             // O motivo de estar a fazer line[5][0] a seguir é porque a função splitLine() inclui o \n no ultimo
             // elemento do array. E, por causa, disso, o nome do grupo estava a ficar numa linha diferente
             line[5][0], resolveGroupName(line[5][0]));
    }

    for (int z = 0; z < camposLidos; z++) free(line[z]);
    free(line);
  }

  printf("\n");

  fclose(f);
}

// Função auxiliar para verificar se um alimento já está incluido numa lista
// A lista está formatada da seguinte maneira: Nome | Valor
bool checkIfItemInArray(char **list, int n, char *nome) {
  for (int i = 0; i < n; i++) {
    char *token = strtok(list[i], " | ");
    if (strcmp(token, nome) == 0) {
      return true;
    }
  }
}

void topAlimentos() {
  short option;
  do {
    printf(
        "Qual é o fator de ordenação?\n"
        " 1 - calorias\n"
        " 2 - proteínas\n"
        " 3 - gorduras\n"
        " 4 - hidratos de carbono\n"
        " 0 - voltar\n");

    scanf("%hd", &option);
  } while (option < 0 || option > 4);

  clearTerminal();

  if (option == 0) return;

  short sortingOrder;
  do {
    printf(
        "Deseja ordenar de forma ascendente ou descendente?\n"
        " 1 - ascendente\n"
        " 2 - descendente\n");

    scanf("%hd", &sortingOrder);
  } while (sortingOrder < 1 || sortingOrder > 2);

  clearTerminal();

  int qnt;
  do {
    printf("Quantos alimentos quer ver?\n");
    scanf("%d", &qnt);
  } while (qnt < 0);

  clearTerminal();

  char **line, **sorted = (char **)malloc(qnt * sizeof(char *)), *str;
  bool setPreviousMaxName = false, setStartingMaxValue;
  int camposLidos, index, currentLine;
  float maxValue;

  FILE *f;

  for (int i = 0; i < qnt; i++) {
    f = openFile(FICHEIRO_ALIMENTOS, "r");

    // Fazer o 'reset' das variaveis auxiliares antes de começar cada iteração
    index = 0;
    currentLine = 0;
    setStartingMaxValue = false;
    maxValue = 0.0;
    char maxName[TAMANHO_NOME];

    while (!feof(f)) {
      line = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");

      currentLine++;

      if (camposLidos < N_CAMPOS_ALIMENTOS) {
        for (int z = 0; z < camposLidos; z++) free(line[z]);
        free(line);
        continue;
      }

      // Ignorar a primeira linha pois é a "legenda" do ficheiro e não contem nenhum alimento.
      if (currentLine != 1) {
        if (!setStartingMaxValue) {
          maxValue = atof(line[option]);
          strcpy(maxName, line[0]);
          setStartingMaxValue = true;
        } else {
          float cur = atof(line[option]);

          // Caso o valor atual seja maior que o valor máximo e o item ainda não estiver na lista, adicionar
          if (cur > maxValue && (i == 0 || checkIfItemInArray(sorted, i, line[0]))) {
            maxValue = cur;
            strcpy(maxName, line[0]);
          }
        }

        index++;
      }

      for (int z = 0; z < camposLidos; z++) free(line[z]);
      free(line);
    }

    str = (char *)malloc(STRING_LENGTH * sizeof(char));

    strcpy(str, maxName);
    strcpy(sorted[i], maxName);
    strcat(sorted[i], " | ");

    char maxValueString[20];

    // Tivemos que converter o valor para uma string, e por isso fizemos uso de uma função auxiliar ftoa()
    // https://www.geeksforgeeks.org/convert-floating-point-number-string/
    ftoa(maxValue, maxValueString);
    strcat(sorted[i], maxValueString);

    free(str);
    fclose(f);
  }

  // 1 - asc, 2 - desc
  if (sortingOrder == 1) {
    for (int i = 0; i < qnt; i++) {
      printf("%s\n", sorted[i]);
    }
  } else {
    for (int i = qnt - 1; i >= 0; i--) {
      printf("%s\n", sorted[i]);
    }
  }

  for (int z = 0; z < qnt; z++) free(sorted[z]);
  free(sorted);
}

#pragma endregion

#pragma region Utentes

// NOME;CC;DIA;MES;ANO;MORADA;LOCALIDADE;CODIGO_POSTAL;TELEFONE;PESO;ALTURA;PRESAO_ARTERIAL_MIN;PRESAO_ARTERIAL_MAX;COLESTEROL_TOTAL;COLESTEROL_HDL;COLESTEROL_LDL

// Abre o ficheiro de utentes consoante o tipo de ficheiro selecionado pelo utilizador
FILE *openUtentesFile(char *txtMode, char *binMode) {
  if (useTextFileForUtentes) return openFile(FICHEIRO_UTENTES_TXT, txtMode);
  return openFile(FICHEIRO_UTENTES_DAT, binMode);
}

/*
 * A maneira de como esta função funciona é:
 *  - cria um ficheiro temporário consoante o tipo de ficheiro selecionado pelo utilizador e vai de linha a linha,
 *    copiando todos os utentes do ficheiro principal para o temporário
 *  - quando encontrar o utento que é para eliminar, simplesmente não vai copiar para o ficheiro temporário
 *  - no final, damos overwrite do ficheiro normal com o conteudo do ficheiro temporário
 *
 * Caso o utente seja encontrado, retorna true. Se não for encontrado, retorna false.
 */
bool deleteUtente(FILE *f, char *nome) {
  FILE *tmpFile;
  if (useTextFileForUtentes) {
    tmpFile = openFile(FICHEIRO_TMP_TXT, "w+");
  } else {
    tmpFile = openFile(FICHEIRO_TMP_DAT, "wb+");
  }

  // Mover o ponteiro do ficheiro para o inicio
  fseek(f, 0, SEEK_SET);

  int camposLidos;
  bool found = false, ignore = false;
  char line[STRING_LENGTH], **info;

  while (!feof(f)) {
    info = splitLine(f, N_CAMPOS_UTENTES, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_UTENTES) {
      for (int z = 0; z < camposLidos; z++) free(info[z]);
      free(info);
      continue;
    }

    for (int i = 0; i < camposLidos; i++) {
      // Se o nome for igual ao do utente para apagar, ignorar
      if (i == 0 && strcmp(info[i], nome) == 0) {
        ignore = true;
        found = true;
      }

      // Contatenar todas as strings que foram separadas pela função splitLine
      if (!ignore) {
        // A função strcat() estava a deixar alguns caracteres nulos no inicio da linha por algum motivo
        if (i == 0) {
          strcpy(line, info[0]);
        } else {
          strcat(line, info[i]);
        }

        // Adicionar o separador, exceto se for o ultimo item
        if (i != N_CAMPOS_UTENTES - 1) {
          strcat(line, ";");
        }
      }
    }

    // Se não for para ignorar, copiar o conteudo da linha para o ficheiro temporário
    if (!ignore) {
      fputs(line, tmpFile);
    }

    // Fazer o 'reset' das variáveis temporárias
    strcpy(line, "");
    ignore = false;
    for (int z = 0; z < camposLidos; z++) free(info[z]);
    free(info);
  }

  fclose(tmpFile);

  // Reescrever o ficheiro de utentes com o conteudo do ficheiro temporário
  if (found) {
    if (useTextFileForUtentes) {
      rename(FICHEIRO_TMP_TXT, FICHEIRO_UTENTES_TXT);
    } else {
      rename(FICHEIRO_TMP_DAT, FICHEIRO_UTENTES_DAT);
    }
  } else {
    if (useTextFileForUtentes) {
      remove(FICHEIRO_TMP_TXT);
    } else {
      remove(FICHEIRO_TMP_DAT);
    }
  }

  return found;
}

void consultarUtentes() {
  FILE *f = openUtentesFile("r+", "rb+");

  char nome[TAMANHO_NOME];

  printf("Qual é o nome do utente que deseja consultar?\n");
  scanf("%s", nome);

  clearTerminal();

  int camposLidos;
  bool found = false;
  char **line;

  while (!feof(f)) {
    line = splitLine(f, N_CAMPOS_UTENTES, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_UTENTES) {
      for (int z = 0; z < camposLidos; z++) free(line[z]);
      free(line);
      continue;
    }

    for (int i = 0; i < camposLidos; i++) {
      if (i == 0 && strcmp(line[i], nome) == 0) {
        // NOME;CC;DIA;MES;ANO;MORADA;LOCALIDADE;CODIGO_POSTAL;TELEFONE;PESO;ALTURA;PRESAO_ARTERIAL_MIN;PRESAO_ARTERIAL_MAX;COLESTEROL_TOTAL;COLESTEROL_HDL;COLESTEROL_LDL
        printf("Nome: %s\n", line[0]);
        printf("Cartão de Cidadão: %s\n", line[1]);
        printf("Data de Nascimento (dd/mm/yyyy): %s/%s/%s\n", line[2], line[3], line[4]);
        printf("Morada: %s\n", line[5]);
        printf("Localidade: %s\n", line[6]);
        printf("Código Postal: %s\n", line[7]);
        printf("Telefone: %s\n", line[8]);
        printf("Peso: %s\n", line[9]);
        printf("Altura: %s\n", line[10]);
        printf("Presão Arterial (min/max): %s/%s\n", line[11], line[12]);
        printf("Colesterol (Total/HDL/LDL): %s/%s/%s\n", line[13], line[14], line[15]);

        found = true;
        break;
      }
    }

    for (int z = 0; z < camposLidos; z++) free(line[z]);
    free(line);
    if (found) break;
  }

  if (!found) {
    printf("Utente '%s' não encontrado.\n", nome);
  }

  fclose(f);
}

void inserirUtente() {
  FILE *f = openUtentesFile("a+", "ab+");

  PESSOA pessoa;

  printf("Qual é o nome do utente?\n");
  scanf("%s", pessoa.nome);

  printf("Qual é o nº do cartão de cidadão do utente?\n");
  scanf("%d", &pessoa.cc);

  do {
    printf("Qual é a data de nascimento do utente (dd/mm/yyyy; e.g. 17/10/2000)?\n");
    scanf("%d/%d/%d", &pessoa.dataNascimento.dia, &pessoa.dataNascimento.mes, &pessoa.dataNascimento.ano);
  } while (!validarData(pessoa.dataNascimento));

  clearTerminal();

  printf("Qual é a morada do utente?\n");
  scanf("%s", pessoa.morada);

  printf("Qual é a localidade do utente?\n");
  scanf("%s", pessoa.localidade);

  printf("Qual é o código postal do utente?\n");
  scanf("%u", &pessoa.codigoPostal);

  clearTerminal();

  printf("Qual é o telefone do utente?\n");
  scanf("%u", &pessoa.telefone);

  printf("Qual é o peso do utente?\n");
  scanf("%u", &pessoa.peso);

  printf("Qual é a altura do utente?\n");
  scanf("%u", &pessoa.altura);

  clearTerminal();

  do {
    printf("Qual é a presão arterial do utente (min/max)?\n");
    scanf("%u/%u", &pessoa.pressaoArterial.min, &pessoa.pressaoArterial.max);
  } while (pessoa.pressaoArterial.min < 0 || pessoa.pressaoArterial.max < 0 ||
           pessoa.pressaoArterial.min > pessoa.pressaoArterial.max);

  printf("Quais são os níveis de colesterol do utente (Total/HDL/LDL)?\n");
  scanf("%d/%d/%d", &pessoa.indiceColesterol.total, &pessoa.indiceColesterol.hdl, &pessoa.indiceColesterol.ldl);

  fprintf(f, "\n%s;%d;%d;%d;%d;%s;%s;%u;%u;%u;%u;%u;%u;%d;%d;%d", pessoa.nome, pessoa.cc, pessoa.dataNascimento.dia,
          pessoa.dataNascimento.mes, pessoa.dataNascimento.ano, pessoa.morada, pessoa.localidade, pessoa.codigoPostal,
          pessoa.telefone, pessoa.peso, pessoa.altura, pessoa.pressaoArterial.min, pessoa.pressaoArterial.max,
          pessoa.indiceColesterol.total, pessoa.indiceColesterol.hdl, pessoa.indiceColesterol.ldl);

  clearTerminal();

  printf("Utente '%s' guardado com sucesso.\n", pessoa.nome);

  fclose(f);
}

void alterarUtente() {
  FILE *f = openUtentesFile("r+", "rb+");

  char nome[TAMANHO_NOME];

  printf("Qual é o nome do utente que deseja alterar?\n");
  scanf("%s", nome);

  int camposLidos;
  bool found = false;
  char **info;

  while (!feof(f)) {
    info = splitLine(f, N_CAMPOS_UTENTES, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_UTENTES) {
      for (int z = 0; z < camposLidos; z++) free(info[z]);
      free(info);
      continue;
    }

    for (int i = 0; i < camposLidos; i++) {
      if (i == 0 && strcmp(info[i], nome) == 0) {
        found = true;
        break;
      }
    }

    if (found) break;
    for (int z = 0; z < camposLidos; z++) free(info[z]);
    free(info);
  }

  if (!found) {
    printf("Utente '%s' não encontrado.\n", nome);
    return;
  }

  short option;
  do {
    printf(
        "O que deseja alterar?\n"
        " 1 - nome\n"
        " 2 - cartão de cidadão\n"
        " 3 - data de nascimento\n"
        " 4 - morada\n"
        " 5 - localidade\n"
        " 6 - código postal\n"
        " 7 - telefone\n"
        " 8 - peso\n"
        " 9 - altura\n"
        "10 - pressão arterial\n"
        "11 - índice de colesterol\n"
        " 0 - voltar\n");

    scanf("%hd", &option);
  } while (option < 0 || option > 11);

  // NOME;CC;DIA;MES;ANO;MORADA;LOCALIDADE;CODIGO_POSTAL;TELEFONE;PESO;ALTURA;PRESAO_ARTERIAL_MIN;PRESAO_ARTERIAL_MAX;COLESTEROL_TOTAL;COLESTEROL_HDL;COLESTEROL_LDL

  char newItem[STRING_LENGTH];
  bool shouldUpdateStr = true;
  int pos = option - 1;

  clearTerminal();

  /*
     0 NOME
     1 CC
     2 DIA
     3 MES
     4 ANO
     5 MORADA
     6 LOCALIDADE
     7 CODIGO_POSTAL
     8 TELEFONE
     9 PESO
    10 ALTURA
    11 PRESSAO_ARTERIAL_MIN
    12 PRESSAO_ARTERIAL_MAX
    13 COLESTEROL_TOTAL
    14 COLESTEROL_HDL
    15 COLESTEROL
  */

  switch (option) {
    case 1:  // Nome
      printf("Qual é o novo nome do utente?\n");
      scanf("%s", newItem);
      break;
    case 2:  // CC
      printf("Qual é o novo nº do cartão de cidadão do utente?\n");
      scanf("%s", newItem);
      break;
    case 3: {  // Data de Nascimento
      char dia[3], mes[3], ano[5];
      printf("Qual é a nova data de nascimento do utente (dd/mm/yyyy; e.g. 17/10/2000)?\n");
      scanf("%s/%s/%s", dia, mes, ano);

      // Como a data está a ser guardada separadamente, vamos ter que atualizar
      // todos os items manualmente
      shouldUpdateStr = false;
      strcpy(info[2], dia);
      strcpy(info[3], mes);
      strcpy(info[4], ano);
      break;
    }
    case 4:  // Morada
      printf("Qual é a nova morada do utente?\n");
      scanf("%s", newItem);
      pos = option + 1;
      break;
    case 5:  // Localidade
      printf("Qual é a nova localidade do utente?\n");
      scanf("%s", newItem);
      pos = option + 1;
      break;
    case 6:  // Código Postal
      printf("Qual é o novo código postal do utente?\n");
      scanf("%s", newItem);
      pos = option + 1;
      break;
    case 7:  // Telefone
      printf("Qual é o novo nº de telefone do utente?\n");
      scanf("%s", newItem);
      pos = option + 1;
      break;
    case 8:  // Peso
      printf("Qual é o novo peso do utente?\n");
      scanf("%s", newItem);
      pos = option + 1;
      break;
    case 9:  // Altura
      printf("Qual é a nova altura do utente?\n");
      scanf("%s", newItem);
      pos = option + 1;
      break;
    case 10: {  // Pressão Arterial
      char min[5], max[5];
      printf("Qual é a nova pressão arterial do utente (min/max)?\n");
      scanf("%s/%s", min, max);

      // Como a data está a ser guardada separadamente, vamos ter que atualizar
      // todos os items manualmente
      shouldUpdateStr = false;
      strcpy(info[11], min);
      strcpy(info[12], max);
      break;
    }
    case 11: {  // Índice de Colesterol
      char total[5], hdl[5], ldl[5];
      printf("Quais são os novos níveis de colesterol do utente (Total/HDL/LDL)?\n");
      scanf("%s/%s/%s", total, hdl, ldl);

      // Como a data está a ser guardada separadamente, vamos ter que atualizar
      // todos os items manualmente
      shouldUpdateStr = false;
      strcpy(info[13], total);
      strcpy(info[14], hdl);
      strcpy(info[15], ldl);
      break;
    }
    case 0:  // Voltar
      for (int z = 0; z < camposLidos; z++) free(info[z]);
      free(info);
      fclose(f);
      return;
  }

  clearTerminal();

  // Se ainda não foi atualizada manualmente, atualizar o item no array
  if (shouldUpdateStr) {
    // Alterar o item que utilizador introduziu
    strcpy(info[pos], newItem);
  }

  // Apagar o utente anterior
  deleteUtente(f, nome);

  // Fechar o ficheiro para abrir em modo de 'append'
  fclose(f);

  f = openUtentesFile("a+", "ab+");

  // Adicionar o utente atualizado
  fprintf(f, "\n%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s", info[0], info[1], info[2], info[3], info[4], info[5],
          info[6], info[7], info[8], info[9], info[10], info[11], info[12], info[13], info[14], info[15]);

  for (int z = 0; z < camposLidos; z++) free(info[z]);
  free(info);
  fclose(f);
}

void eliminarUtente() {
  FILE *f = openUtentesFile("r+", "rb+");

  char nome[TAMANHO_NOME];

  printf("Qual é o nome do utente que quer eliminar?\n");
  scanf("%s", nome);

  clearTerminal();

  if (deleteUtente(f, nome)) {
    printf("O utente '%s' foi eliminado com sucesso.\n", nome);
  } else {
    printf("Não foi possível eliminar o utente com o nome '%s'.\n", nome);
  }

  fclose(f);
}

#pragma endregion

#pragma region Menus

/*
 * 1. Pequeno Almoço; (1, 3, 6)
 * 2. Almoco; (1, 2, 4, 5, 6, 7, 8)
 * 3. Lanche; (1, 3, 6)
 * 4. Jantar; (1, 2, 4, 5, 6, 7, 8)
 * 5. Ceia; (1, 3, 6)
 * 6. Outra. (1, 2, 3, 4, 5, 6, 7, 8)
 */
bool alimentoIncluidoNoTipoDeRefeicao(int grupoAlimento, int tipoDeRefeicao) {
  switch (tipoDeRefeicao) {
    case 1:
    case 3:
    case 5:
      return grupoAlimento == 1 || grupoAlimento == 3 || grupoAlimento == 6;
    case 2:
    case 4:
      return grupoAlimento == 1 || grupoAlimento == 2 || (grupoAlimento >= 4 && grupoAlimento <= 8);
    case 6:
      return grupoAlimento >= 1 && grupoAlimento <= 8;
    default:
      return false;
  }
}

void askForBaseMenu(MENU *menu) {
  printf("Qual é o número de cartão de cidadão do utente?\n");
  scanf("%u", &menu->cc);

  do {
    printf("Qual é a data da refeição (dd/mm/yyyy; e.g. 3/4/2022)?\n");
    scanf("%d/%d/%d", &menu->dataRefeicao.dia, &menu->dataRefeicao.mes, &menu->dataRefeicao.ano);
  } while (!validarData(menu->dataRefeicao));

  clearTerminal();

  do {
    printf(
        "Qual é o tipo de refeição?\n"
        " 1 - Pequeno Almoço\n"
        " 2 - Almoço\n"
        " 3 - Lanche\n"
        " 4 - Jantar\n"
        " 5 - Ceia\n"
        " 6 - Outra\n");

    scanf("%hd", &menu->refeicaoDoDia);
  } while (menu->refeicaoDoDia < 1 || menu->refeicaoDoDia > 6);

  clearTerminal();

  printf("Qual é a quantidade de calorias que pretende ingerir (KCal)?\n");
  scanf("%f", &menu->kCalorias);

  printf("Qual é o mínimo e máximo de calorias que pretende ingerir (min/max)?\n");
  scanf("%f/%f", &menu->minProteinas, &menu->maxProteinas);

  clearTerminal();

  printf("Qual é a quantidade de gorduras?\n");
  scanf("%f", &menu->gorduras);

  printf("Qual é a quantidade de hidratos de carbono?\n");
  scanf("%f", &menu->hidratosCarbono);

  clearTerminal();
}

void gerarMenu() {
  MENU menu;

  askForBaseMenu(&menu);

  short minimoCategorias;
  do {
    printf("Qual é o minimo de categorias de alimentos a incluir na refeição?\n");
    scanf("%hd", &minimoCategorias);
  } while (minimoCategorias < 1 || minimoCategorias > 10);

  clearTerminal();

  FILE *f = openFile(FICHEIRO_ALIMENTOS, "r");

  int camposLidos, totalCalorias = 0, totalProteinas = 0, totalGorduras = 0, totalHidratosCarbono = 0, categorias = 0;
  char **line;

  /*
    Gerar um conjunto de 10 refeições do tipo indicado pelo utilizador e com as
    calorias que pretende ingerir, atendendo igualmente ao mínimo e máximo de
    Proteínas, Gorduras e H. Carbono especificados e ao mínimo de categorias de
    alimentos a incluir, devendo, cada proposta, aproximarse ao máximo das
    especificações indicadas pela pessoa;
  */

  for (int i = 0; i < N_REFEICOES; i++) {
    // Mover o ponteiro para o inicio do ficheiro em cada iteração
    fseek(f, 0, SEEK_SET);

    int qntAlimentos = 0;

    printf("--- %dª Refeição ---\n", i + 1);

    while (!feof(f)) {
      if (qntAlimentos >= MAX_ALIMENTOS) {
        break;
      }

      line = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");
      if (camposLidos < N_CAMPOS_ALIMENTOS) {
        for (int z = 0; z < camposLidos; z++) free(line[z]);
        free(line);
        continue;
      }

      // O motivo de usarmos isto é porque se não usarmos, todos os menus vão ter sempre os mesmos alimentos
      // Como funciona: rand() genera um número aleatorio entre 0 e RAND_MAX (+32k)
      // Neste caso, usamos o operador de modulo para obter o resto da divisão por 100, que irá dar um int entre 0 e 99
      // Caso esse int seja maior ou igual 50, vamos passar ao proximo alimento
      // Fazendo assim uma chance de 50% de o alimento ser escolhido
      // https://stackoverflow.com/a/822368/11252146
      if (rand() % 100 >= 50) {
        continue;
      }

      // O motivo de usarmos isto é porque temos que comparar 2 números, e como os valores lidos do ficheiro são strings
      // Não conseguimos fazer essa comparação.
      // atoi(): Função para converter string para int
      // https://www.educative.io/edpresso/how-to-convert-a-string-to-an-integer-in-c
      int grupo = atoi(line[5]);

      if (alimentoIncluidoNoTipoDeRefeicao(grupo, menu.refeicaoDoDia)) {
        int calorias = atoi(line[2]);

        // Tal como explicado em cima, temos que usar esta função pois os valores lidos do ficheiro são strings
        // atof(): Função para converter string para float (essencialmente o mesmo que o atoi(), mas para floats)
        // https://stackoverflow.com/a/7951034/11252146
        float proteina = atof(line[3]), gordura = atof(line[4]), hidratosCarbono = atof(line[6]);

        // Caso ultrapasse um dos limites, passar ao próximo alimento
        if ((totalCalorias + calorias > menu.kCalorias) || (totalProteinas + proteina > menu.maxProteinas) ||
            (totalGorduras + gordura > menu.gorduras) ||
            (totalHidratosCarbono + hidratosCarbono > menu.hidratosCarbono)) {
          for (int z = 0; z < camposLidos; z++) free(line[z]);
          free(line);
          continue;
        }

        strcpy(menu.refeicoes[i].alimentos[qntAlimentos].nome, line[0]);
        menu.refeicoes[i].alimentos[qntAlimentos].quantidade = 1;

        printf(" - %d | %s\n", qntAlimentos + 1, line[0]);

        qntAlimentos++;
        totalCalorias += calorias;
        totalProteinas += proteina;
        totalGorduras += gordura;
        totalHidratosCarbono += hidratosCarbono;
      }

      for (int z = 0; z < camposLidos; z++) free(line[z]);
      free(line);
    }

    printf("--------------------------\n");
  }

  fclose(f);
}

bool alimentoExiste(char *nome, int *calorias, float *proteinas, float *gorduras, float *hidratosCarbono, int *grupo) {
  FILE *f = openFile(FICHEIRO_ALIMENTOS, "r");

  int camposLidos;
  char **line;
  bool found = false;

  *calorias = 0;
  *proteinas = 0.0;
  *gorduras = 0.0;
  *hidratosCarbono = 0.0;
  *grupo = 8;

  while (!feof(f)) {
    line = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_ALIMENTOS) {
      for (int z = 0; z < camposLidos; z++) free(line[z]);
      free(line);
      continue;
    }

    if (strcmp(line[0], nome) == 0) {
      *calorias = atoi(line[1]);
      *proteinas = atof(line[2]);
      *gorduras = atof(line[3]);
      *hidratosCarbono = atof(line[4]);
      *grupo = atoi(line[5]);
      found = true;
      break;
    }

    for (int z = 0; z < camposLidos; z++) free(line[z]);
    free(line);
  }

  fclose(f);

  return found;
}

bool introduziuSair(char *nome) {
  return strcmp(nome, "sair") == 0 || strcmp(nome, "'sair'") == 0;
}

void guardarMenu(MENU menu) {
  FILE *f = openFile(FICHEIRO_MENUS_DAT, "ab+");

  fprintf(f, "\n%u;%hd;%d;%d;%d;%f;%f;%f;%f;%f;", menu.cc, menu.refeicaoDoDia, menu.dataRefeicao.dia,
          menu.dataRefeicao.mes, menu.dataRefeicao.ano, menu.kCalorias, menu.minProteinas, menu.maxProteinas,
          menu.gorduras, menu.hidratosCarbono);

  for (int i = 0; i < N_REFEICOES; i++) {
    for (int j = 0; j < MAX_ALIMENTOS; j++) {
      char format[7] = "%s;%d;";
      if (i == N_REFEICOES - 1 && j == MAX_ALIMENTOS - 1) {
        strcpy(format, "%s;%d");
      }
      fprintf(f, format, menu.refeicoes[i].alimentos[j].nome, menu.refeicoes[i].alimentos[j].quantidade);
    }
  }

  fclose(f);
}

// 1-Peq. Almoço, 2-Almoço, 3-Lanche, 4-Jantar, 5-Ceia, 6-Outro
char *resolveRefeicaoDoDia(unsigned short refeicaoDoDia) {
  switch (refeicaoDoDia) {
    case 1:
      return "Pequeno Almoço";
    case 2:
      return "Almoço";
    case 3:
      return "Lanche";
    case 4:
      return "Jantar";
    case 5:
      return "Ceia";
    case 6:
      return "Outro";
    default:
      return "";
  }
}

void criarMenu() {
  MENU menu;
  char nome[TAMANHO_NOME];
  int i, quantidade, alimentos;

  askForBaseMenu(&menu);

  int calorias, grupo, totalCalorias = 0;
  float proteinas, gorduras, hidratosCarbono, totalProteinas = 0, totalGorduras = 0, totalHidratosCarbono = 0;

  for (int j = 0; j < N_REFEICOES; j++) {
    for (i = 0; i < MAX_ALIMENTOS; i++) {
      clearTerminal();

      printf("--- %dª Refeição ---\n\n", j + 1);

      do {
        printf("Introduza o nome de um alimento que pretende ingerir.\n");
        printf("Para sair, introduza 'sair'.\n");

        scanf("%s", nome);
      } while (!alimentoExiste(nome, &calorias, &proteinas, &gorduras, &hidratosCarbono, &grupo) &&
               !introduziuSair(nome));

      if (introduziuSair(nome)) {
        break;
      }

      do {
        printf("Qual é a quantidade de '%s' que pretende ingerir?\n", nome);
        scanf("%d", &quantidade);
      } while (quantidade < 0 || quantidade > MAX_ALIMENTOS - alimentos);

      totalCalorias += calorias * quantidade;
      totalProteinas += proteinas * quantidade;
      totalGorduras += gorduras * quantidade;
      totalHidratosCarbono += hidratosCarbono * quantidade;

      strcpy(menu.refeicoes[j].alimentos[alimentos].nome, nome);
      menu.refeicoes[j].alimentos[alimentos].quantidade = quantidade;

      alimentos++;
    }
  }

  // display the menu to the user
  clearTerminal();

  printf("----------- Menu -----------\n");
  printf("Data: %d/%d/%d\n", menu.dataRefeicao.dia, menu.dataRefeicao.mes, menu.dataRefeicao.ano);
  printf("CC: %u\n", menu.cc);
  printf("Refeição do dia: %s\n", resolveRefeicaoDoDia(menu.refeicaoDoDia));
  printf("Alimentos:\n");

  for (int i = 0; i < N_REFEICOES; i++) {
    printf("\n--- %dª Refeição ---\n\n", i + 1);
    for (int j = 0; j < MAX_ALIMENTOS; j++) {
      printf(" - %d | %s\n", j + 1, menu.refeicoes[i].alimentos[j].nome);
    }
  }

  printf("\n--------------------------\n");
  printf("Total Calorias: %d\n", totalCalorias);
  printf("Total Proteínas: %.2f\n", totalProteinas);
  printf("Total Gorduras: %.2f\n", totalGorduras);
  printf("Total Hidratos de Carbono: %.2f\n", totalHidratosCarbono);
  printf("--------------------------\n");

  guardarMenu(menu);
}

#pragma endregion

#pragma region InteractiveMenu

// Isto é necessário porque as duas funções se chamam uma à outra em certas
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
        " 5 - listar\n"
        " 6 - mostrar top\n"
        " 0 - voltar\n");
    scanf("%hd", &option);
  } while (option > 6 || option < 0);

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
      eliminarAlimento();
      break;
    case 5:
      listarAlimentos();
      break;
    case 6:
      topAlimentos();
      break;
    case 0:  // voltar
      startInteractiveMenu();
      return;
  }
}

void startUtentesInteractiveMenu() {
  short option;
  do {
    clearTerminal();

    printf(
        "O que deseja fazer aos utentes?\n"
        " 1 - consultar\n"
        " 2 - inserir\n"
        " 3 - alterar\n"
        " 4 - eliminar\n"
        " 0 - voltar\n");
    scanf("%hd", &option);
  } while (option > 4 || option < 0);

  clearTerminal();

  switch (option) {
    case 1:
      consultarUtentes();
      break;
    case 2:
      inserirUtente();
      break;
    case 3:
      alterarUtente();
      break;
    case 4:
      eliminarUtente();
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
        " 3 - gerar menu (automático)\n"
        " 4 - criar menu (manual)\n"
        " 0 - sair\n");
    scanf("%hd", &option);
  } while (option > 4 || option < 0);

  clearTerminal();

  switch (option) {
    case 1:  // alimentos
      startAlimentosInteractiveMenu();
      break;

    case 2: {  // utentes
      char tipoFicheiro[4];
      do {
        printf("Qual é o tipo de ficheiro que deseja usar? (txt ou dat) ");
        scanf("%s", tipoFicheiro);
      } while (strcmp(tipoFicheiro, "txt") != 0 && strcmp(tipoFicheiro, "dat") != 0);

      if (strcmp(tipoFicheiro, "txt") == 0) {
        useTextFileForUtentes = true;
      }

      startUtentesInteractiveMenu();
      break;
    }

    case 3:  // gerar menu (automático)
      gerarMenu();
      break;

    case 4:  // criar menu (manual)
      criarMenu();
      break;

    case 0:
      exit(0);
      return;
  }
}

#pragma endregion

void main() {
  setlocale(LC_ALL, "Portuguese");

  // Inicializa o generador de números aleatórios que está a ser usado na geração de menus, na função gerarMenu()
  // https://stackoverflow.com/a/822368/11252146
  srand(time(NULL));

  startInteractiveMenu();
}
