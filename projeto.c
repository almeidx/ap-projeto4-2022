#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// Função de utilidade para limpar o terminal
void clearTerminal() {
#ifdef _WIN32  // Windows
  system("cls");
#else  // Se não
  system("clear");
#endif
}

// Função de utilidade para dar free à memória de um array de strings
void freeArrayOfStrings(char **str, int n) {
  for (int i = 0; i < n; i++) free(str[n]);
  free(str);
}

// Usar #pragma para definir regiões para ser mais fácil navegar o ficheiro
// (pode não ser suportado em todos os IDEs)
// https://stackoverflow.com/a/9000516/11252146
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

/*
 * A maneira de como esta função funciona é:
 *  - cria um ficheiro temporário e vai de linha a linha, copiando todos os
 * alimentos do ficheiro principal para o temporário
 *  - quando encontrar o alimento que é para eliminar, simplesmente não vai
 * copiar para o ficheiro temporário
 *  - no final, damos overwrite do ficheiro normal com o conteudo do ficheiro
 * temporário
 *
 * Caso o alimento seja encontrado, retorna true. Se não for encontrado, retorna
 * false.
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
      freeArrayOfStrings(info, camposLidos);
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
    freeArrayOfStrings(info, camposLidos);
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
      freeArrayOfStrings(line, camposLidos);
      continue;
    }

    for (int i = 0; i < camposLidos; i++) {
      if (i == 0 && strcmp(line[i], nome) == 0) {
        printf("Nome: %s\n", line[0]);
        printf("Calorias (KCal): %s\n", line[1]);
        printf("Proteínas (g): %s\n", line[2]);
        printf("Gorduras (g): %s\n", line[3]);
        printf("Hidratos de Carbono (g): %s\n", line[4]);
        printf("Grupo: %s\n", line[5]);

        found = true;
        break;
      }
    }

    freeArrayOfStrings(line, camposLidos);
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
  float calorias, proteinas, gordura, hidratosCarbono;

  printf("Qual é o nome do alimento?\n");
  scanf("%s", nome);

  printf("Qual é o grupo do alimento?\n");
  scanf("%d", &grupo);

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
      freeArrayOfStrings(info, camposLidos);
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

    freeArrayOfStrings(info, camposLidos);
  }

  if (!found) {
    printf("Alimento '%s' não encontrado.\n", nome);
    return;
  }

  short option;
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
  f = openFile(FICHEIRO_ALIMENTOS, "a");

  // Adicionar o alimento atualizado
  fprintf(f, "\n%s;%s;%s;%s;%s;%s", info[0], info[1], info[2], info[3], info[4], info[5]);

  freeArrayOfStrings(info, camposLidos);
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

  int camposLidos;
  char **line;

  clearTerminal();

  while (!feof(f)) {
    line = splitLine(f, N_CAMPOS_ALIMENTOS, &camposLidos, ";");
    if (camposLidos < N_CAMPOS_ALIMENTOS) {
      for (int i = 0; i < camposLidos; i++) free(line[i]);
      free(line);
      continue;
    }

    printf("\n%s | %s | %s | %s | %s | %s", line[0], line[1], line[2], line[3], line[4], line[5]);

    for (int i = 0; i < camposLidos; i++) free(line[i]);
    free(line);
  }

  printf("\n");

  fclose(f);
}

#pragma endregion

#pragma region Utentes

// NOME;CC;DIA;MES;ANO;MORADA;LOCALIDADE;CODIGO_POSTAL;TELEFONE;PESO;ALTURA;PRESAO_ARTERIAL_MIN;PRESAO_ARTERIAL_MAX;COLESTEROL_TOTAL;COLESTEROL_HDL;COLESTEROL_LDL

// Abre o ficheiro de utentes consoante o tipo de ficheiro selecionado pelo
// utilizador
FILE *openUtentesFile(char *txtMode, char *binMode) {
  if (useTextFileForUtentes) return openFile(FICHEIRO_UTENTES_TXT, txtMode);
  return openFile(FICHEIRO_UTENTES_DAT, binMode);
}

/*
 * A maneira de como esta função funciona é:
 *  - cria um ficheiro temporário consoante o tipo de ficheiro selecionado pelo
 * utilizador e vai de linha a linha, copiando todos os utentes do ficheiro
 * principal para o temporário
 *  - quando encontrar o utento que é para eliminar, simplesmente não vai copiar
 *    para o ficheiro temporário
 *  - no final, damos overwrite do ficheiro normal com o conteudo do ficheiro
 * temporário
 *
 * Caso o utente seja encontrado, retorna true. Se não for encontrado, retorna
 * false.
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
      freeArrayOfStrings(info, camposLidos);
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
        // A função strcat() estava a deixar alguns caracteres nulos no inicio
        // da linha por algum motivo
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

    // Se não for para ignorar, copiar o conteudo da linha para o ficheiro
    // temporário
    if (!ignore) {
      fputs(line, tmpFile);
    }

    // Fazer o 'reset' das variáveis temporárias
    strcpy(line, "");
    ignore = false;
    freeArrayOfStrings(info, camposLidos);
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
      freeArrayOfStrings(line, camposLidos);
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

    freeArrayOfStrings(line, camposLidos);
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
    printf(
        "Qual é a data de nascimento do utente (dd/mm/yyyy; e.g. "
        "17/10/2000)?\n");
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
      freeArrayOfStrings(info, camposLidos);
      continue;
    }

    for (int i = 0; i < camposLidos; i++) {
      if (i == 0 && strcmp(info[i], nome) == 0) {
        found = true;
        break;
      }
    }

    if (found) break;
    freeArrayOfStrings(info, camposLidos);
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
        "11 - indice de colesterol\n"
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
      printf(
          "Qual é a nova data de nascimento do utente (dd/mm/yyyy; e.g. "
          "17/10/2000)?\n");
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
      printf(
          "Quais são os novos níveis de colesterol do utente "
          "(Total/HDL/LDL)?\n");
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
      freeArrayOfStrings(info, camposLidos);
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

  freeArrayOfStrings(info, camposLidos);
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

// TODO
// void guardarMenu(MENU menu) {
//   FILE *f = openFile(FICHEIRO_MENUS_DAT, "a+");

//   menu.refeicoes

//   // cc;refeicao_do_dia;data_dia;data_mes;data_ano;gorduras;hidratos_carbono;calorias;min_protainas;max_protainas

//   fprintf(
//     f,
//     "\n%u;",
//     menu.cc
//   );
// }

void gerarMenu() {
  MENU menu;

  printf("Qual é o número de cartão de cidadão da pessoa?\n");
  scanf("%u", &menu.cc);

  do {
    printf("Qual é a data da refeição (dd/mm/yyyy; e.g. 3/4/2022)?\n");
    scanf("%d/%d/%d", &menu.dataRefeicao.dia, &menu.dataRefeicao.mes, &menu.dataRefeicao.ano);
  } while (!validarData(menu.dataRefeicao));

  clearTerminal();

  do {
    printf(
        "Qual é o tipo de refeição que quer?\n"
        " 1 - Pequeno Almoço\n"
        " 2 - Almoço\n"
        " 3 - Lanche\n"
        " 4 - Jantar\n"
        " 5 - Ceia\n"
        " 6 - Outra\n");

    scanf("%hd", &menu.refeicaoDoDia);
  } while (menu.refeicaoDoDia < 1 || menu.refeicaoDoDia > 6);

  clearTerminal();

  printf("Qual é a quantidade de calorias que pretende ingerir (KCal)?\n");
  scanf("%f", &menu.kCalorias);

  printf("Qual é o mínimo e máximo de calorias que pretende ingerir (min/max)?\n");
  scanf("%f/%f", &menu.minProteinas, &menu.maxProteinas);

  clearTerminal();

  printf("Qual é a quantidade de gorduras?\n");
  scanf("%f", &menu.gorduras);

  printf("Qual é a quantidade de hidratos de carbono?\n");
  scanf("%f", &menu.hidratosCarbono);

  clearTerminal();

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
        freeArrayOfStrings(line, camposLidos);
        continue;
      }

      // rand() genera um número aleatorio entre 0 e RAND_MAX (+32k)
      // https://stackoverflow.com/a/822368/11252146
      if (rand() % 100 > 50) {
        continue;
      }

      // atoi(): Função para converter string para int
      // https://www.educative.io/edpresso/how-to-convert-a-string-to-an-integer-in-c
      int grupo = atoi(line[5]);

      if (alimentoIncluidoNoTipoDeRefeicao(grupo, menu.refeicaoDoDia)) {
        int calorias = atoi(line[2]);

        // atof(): Função para converter string para float (basicamente o mesmo que o atoi())
        // https://stackoverflow.com/a/7951034/11252146
        float proteina = atof(line[3]), gordura = atof(line[4]), hidratosCarbono = atof(line[6]);

        // Caso ultrapasse um dos limites, passar ao próximo alimento
        if ((totalCalorias + calorias > menu.kCalorias) || (totalProteinas + proteina > menu.maxProteinas) ||
            (totalGorduras + gordura > menu.gorduras) ||
            (totalHidratosCarbono + hidratosCarbono > menu.hidratosCarbono)) {
          freeArrayOfStrings(line, camposLidos);
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

      freeArrayOfStrings(line, camposLidos);
    }

    printf("--------------------------\n");
  }

  fclose(f);
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
        " 0 - voltar\n");
    scanf("%hd", &option);
  } while (option > 5 || option < 0);

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
        " 3 - gerar refeições\n"
        " 0 - sair\n");
    scanf("%hd", &option);
  } while (option > 3 || option < 0);

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

    case 3:  // gerar refeições
      gerarMenu();
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
