#include "io_manager.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* Sprawdza czy dany system operacyjny/procesor operuje w formacie Little-Endian.
 * Zwraca wartość większą od zera (true), jeśli tak. */
int isLittleEndian(void)
{
  uint16_t test = 0x0001;
  uint8_t byte;
  memcpy(&byte, &test, 1);
  return byte == 0x01;
}

/* Konwertuje 32-bitową liczbę z formatu natywnego (host) na format Big-Endian.
 * Pamięć jest przemieszczana za pomocą tymczasowych tablic bajtów. */
uint32_t toBigEndianUint32(uint32_t val)
{
  if (!isLittleEndian())
  {
    return val;
  }
  uint8_t in[4] = {0};
  uint8_t out[4] = {0};
  uint32_t result = 0;
  memcpy(in, &val, 4);
  for (int i = 0; i < 4; i++)
  {
    out[i] = in[3 - i];
  }
  memcpy(&result, out, 4);
  return result;
}

/* Konwertuje 64-bitową liczbę zmiennoprzecinkową z formatu natywnego (host)
 * na format Big-Endian poprzez zmianę kolejności ośmiu bajtów. */
double toBigEndianDouble(double val)
{
  if (!isLittleEndian())
  {
    return val;
  }
  uint8_t in[8] = {0};
  uint8_t out[8] = {0};
  double result = 0.0;
  memcpy(in, &val, 8);
  for (int i = 0; i < 8; i++)
  {
    out[i] = in[7 - i];
  }
  memcpy(&result, out, 8);
  return result;
}

typedef struct
{
  char **names;
  int count;
  int capacity;
} VertexList;

/* Inicjalizuje pustą listę wierzchołków, prealokując początkową pojemność. */
int initVertexList(VertexList *list)
{
  list->count = 0;
  list->capacity = 10;
  list->names = malloc(list->capacity * sizeof(char *));
  if (list->names == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia listy wierzchołków!\n");
    return -1;
  }
  return 0;
}

/* Dodaje unikalną nazwę wierzchołka do tymczasowej listy odczytu.
 * Zwraca indeks istniejącego lub nowo dodanego wierzchołka w liście, albo -1 na wypadek błędu. */
int addUniqueVertex(VertexList *list, const char *name)
{
  // Sprawdź czy już istnieje
  for (int i = 0; i < list->count; i++)
  {
    if (strcmp(list->names[i], name) == 0)
      return i;
  }

  // Jeśli brak miejsca powiększ tablicę
  if (list->count >= list->capacity)
  {
    int newCapacity = list->capacity * 2;
    char **tmp = realloc(list->names, newCapacity * sizeof(char *));
    if (tmp == NULL)
    {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas rozszerzania tablicy nazw wierzchołków!\n");
      return -1;
    }
    list->names = tmp;
    list->capacity = newCapacity;
  }

  list->names[list->count] = strdup(name);
  if (list->names[list->count] == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas kopiowania nazwy wierzchołka!\n");
    return -1;
  }
  return list->count++;
}

/* Czyści zasoby zaalokowane przez listę wierzchołków (zwalnia stringi i samą listę). */
void cleanupVertexList(VertexList *list)
{
  for (int i = 0; i < list->count; i++)
    free(list->names[i]);
  free(list->names);
}

/* Wczytuje i parsuje krawędzie z podanego pliku wejściowego.
 * Zapisuje wynik do wyjściowej tablicy krawędzi i aktualizuje listę unikalnych wierzchołków. */
static int readEdges(FILE *inputFile, Edge **edgesOut, int *countOut, VertexList *vList)
{
  char buff[4096];
  int capacity = 16;
  int count = 0;
  Edge *edges = malloc(capacity * sizeof(Edge));
  if (!edges)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia tablicy krawędzi!\n");
    return -1;
  }

  char tempName[256] = {0};
  char nameA[256] = {0};
  char nameB[256] = {0};
  double weight = 0.0;

  while (fgets(buff, sizeof(buff), inputFile))
  {
    if (buff[0] == '\n' || buff[0] == '#')
      continue;

    if (strchr(buff, '\n') == NULL && !feof(inputFile))
    {
      fprintf(stderr, "Błąd! Linia w pliku wejściowym jest zbyt długa!\n");
      for (int i = 0; i < count; i++)
        free(edges[i].name);
      free(edges);
      return -1;
    }

    if (count >= capacity)
    {
      capacity *= 2;
      Edge *tmp = realloc(edges, capacity * sizeof(Edge));
      if (!tmp)
      {
        fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas rozszerzania tablicy krawędzi!\n");
        for (int i = 0; i < count; i++)
          free(edges[i].name);
        free(edges);
        return -1;
      }
      edges = tmp;
    }

    if (sscanf(buff, "%255s %255s %255s %lf", tempName, nameA, nameB, &weight) != 4)
    {
      fprintf(stderr, "Błąd! Linia w pliku nie pasuje do formatu: <nazwa> <idA> <idB> <waga>!\n");
      for (int i = 0; i < count; i++)
        free(edges[i].name);
      free(edges);
      return -1;
    }

    if (!isfinite(weight) || weight < 0)
    {
      fprintf(stderr, "Błąd! Waga krawędzi musi być nieujemną liczbą skończoną!\n");
      for (int i = 0; i < count; i++)
        free(edges[i].name);
      free(edges);
      return -1;
    }

    int idA = addUniqueVertex(vList, nameA);
    if (idA == -1)
    {
      for (int i = 0; i < count; i++)
        free(edges[i].name);
      free(edges);
      return -1;
    }
    int idB = addUniqueVertex(vList, nameB);
    if (idB == -1)
    {
      for (int i = 0; i < count; i++)
        free(edges[i].name);
      free(edges);
      return -1;
    }

    if (idA == idB)
    {
      fprintf(stderr, "Ostrzeżenie: Wykryto pętlę własną dla wierzchołka %s\n", nameA);
    }

    edges[count].idA = idA;
    edges[count].idB = idB;
    edges[count].name = strdup(tempName);
    if (edges[count].name == NULL)
    {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas kopiowania nazwy krawędzi!\n");
      for (int i = 0; i < count; i++)
        free(edges[i].name);
      free(edges);
      return -1;
    }
    count++;
  }

  if (count == 0)
  {
    fprintf(stderr, "Błąd! Liczba wczytanych krawędzi jest równa 0!\n");
    free(edges);
    return -1;
  }

  *edgesOut = edges;
  *countOut = count;
  return 0;
}

/* Pomocnicza funkcja inicjalizująca tablice i inne pola wewnątrz struktury grafu
 * w oparciu o znaną liczbę wierzchołków i krawędzi. */
static int allocateGraph(Graph *graph, int vCount, int eCount)
{
  graph->verticesCount = vCount;
  graph->edgesCount = eCount;
  graph->edges = NULL;
  graph->x = NULL;
  graph->y = NULL;
  graph->dx = NULL;
  graph->dy = NULL;

  graph->vertices = calloc(vCount, sizeof(Node));
  graph->x = malloc(vCount * sizeof(double));
  graph->y = malloc(vCount * sizeof(double));
  graph->dx = calloc(vCount, sizeof(double));
  graph->dy = calloc(vCount, sizeof(double));

  if (!graph->vertices || !graph->x || !graph->y || !graph->dx || !graph->dy)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas przydzielania współrzędnych i grafu!\n");
    freeGraph(graph);
    return -1;
  }
  return 0;
}

/* Główna funkcja ładująca definicję grafu z otwartego strumienia pliku.
 * Obsługuje inicjalizację pomocniczych struktur i tworzenie wierzchołków
 * oraz przypisuje początkowe losowe współrzędne x/y w podanym wymiarze. */
int loadGraph(FILE *inputFile, Graph *graph, int width, int height)
{
  VertexList vList;
  if (initVertexList(&vList) != 0)
  {
    return -1;
  }
  Edge *tempEdges = NULL;
  int edgesCount = 0;

  if (readEdges(inputFile, &tempEdges, &edgesCount, &vList) != 0)
  {
    cleanupVertexList(&vList);
    return -1;
  }

  if (vList.count == 0)
  {
    fprintf(stderr, "Błąd! Plik nie zawiera żadnych wierzchołków! \n");
    cleanupVertexList(&vList);
    free(tempEdges);
    return -1;
  }

  if (allocateGraph(graph, vList.count, edgesCount) != 0)
  {
    for (int i = 0; i < edgesCount; i++)
    {
      free(tempEdges[i].name);
    }
    free(tempEdges);
    cleanupVertexList(&vList);
    return -1;
  }
  graph->edges = tempEdges;
  for (int i = 0; i < edgesCount; i++)
  {
    if (addVertex(graph->vertices, graph->edges[i].idA, graph->edges[i].idB) == -1 ||
        addVertex(graph->vertices, graph->edges[i].idB, graph->edges[i].idA) == -1)
    {
      cleanupVertexList(&vList);
      freeGraph(graph);
      return -1;
    }
  }

  for (int i = 0; i < graph->verticesCount; i++)
  {
    graph->x[i] = (double)rand() / RAND_MAX * width;
    graph->y[i] = (double)rand() / RAND_MAX * height;
    graph->vertices[i].name = strdup(vList.names[i]);
    if (graph->vertices[i].name == NULL)
    {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas kopiowania nazwy!\n");
      cleanupVertexList(&vList);
      freeGraph(graph);
      return -1;
    }
  }

  cleanupVertexList(&vList);
  return 0;
}

/* Zapisuje ułożony graf (nazwy oraz współrzędne węzłów) do strumienia wyjściowego.
 * Może zapisać dane w czytelnym formacie tekstowym, lub w binarnym, odpowiednio
 * obsługując wtedy system Endian. */
void saveResults(FILE *outputFile, Graph *graph, bool isBinary)
{
  if (isBinary == false)
  {
    for (int i = 0; i < graph->verticesCount; i++)
    {
      fprintf(outputFile, "%s %g %g\n", graph->vertices[i].name, graph->x[i],
              graph->y[i]);
    }
  }
  else
  {
    for (int i = 0; i < graph->verticesCount; i++)
    {
      uint32_t nameLen = strlen(graph->vertices[i].name);
      uint32_t lenBe = toBigEndianUint32(nameLen);
      double xBe = toBigEndianDouble(graph->x[i]);
      double yBe = toBigEndianDouble(graph->y[i]);

      fwrite(&lenBe, sizeof(uint32_t), 1, outputFile);
      fwrite(graph->vertices[i].name, sizeof(char), nameLen, outputFile);
      fwrite(&xBe, sizeof(double), 1, outputFile);
      fwrite(&yBe, sizeof(double), 1, outputFile);
    }
  }
}

/* Analizuje argumenty wiersza poleceń uruchomienia programu za pomocą funkcji getopt.
 * Zapisuje poprawne argumenty do struktury CliFlags. Zwraca -1 gdy użyto błędnej flagi. */
int parseCliFlags(int argc, char *argv[], CliFlags *flags)
{
  int opt = 0;
  long parsedValue = 0.0;
  char *endPtr = NULL;

  flags->width = 1000;
  flags->height = 1000;
  flags->iter = 100;
  flags->seed = 0;
  flags->isBinary = false;
  flags->isText = false;
  flags->isSeedSet = false;
  flags->inputFile = NULL;
  flags->outputFile = NULL;
  flags->algorithmName = NULL;

  while ((opt = getopt(argc, argv, "i:o:w:h:t:a:b:s:")) != -1)
  {
    switch (opt)
    {
    case 'i':
      flags->inputFile = optarg;
      break;
    case 'o':
      flags->isText = true;
      flags->outputFile = optarg;
      break;
    case 'w':
      errno = 0;
      parsedValue = strtol(optarg, &endPtr, 10);
      if (errno != 0 || *endPtr != '\0')
      {
        fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -w: %s!\n", optarg);
        return -1;
      }
      flags->width = (int)parsedValue;
      break;
    case 'h':
      errno = 0;
      parsedValue = strtol(optarg, &endPtr, 10);
      if (errno != 0 || *endPtr != '\0')
      {
        fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -h: %s!\n", optarg);
        return -1;
      }
      flags->height = (int)parsedValue;
      break;
    case 't':
      errno = 0;
      parsedValue = strtol(optarg, &endPtr, 10);
      if (errno != 0 || *endPtr != '\0')
      {
        fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -t: %s!\n", optarg);
        return -1;
      }
      flags->iter = (int)parsedValue;
      break;
    case 'a':
      flags->algorithmName = optarg;
      break;
    case 'b':
      flags->isBinary = true;
      flags->outputFile = optarg;
      break;
    case 's':
      errno = 0;
      parsedValue = strtol(optarg, &endPtr, 10);
      if (errno != 0 || *endPtr != '\0')
      {
        fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -s: %s!\n", optarg);
        return -1;
      }
      flags->seed = (int)parsedValue;
      flags->isSeedSet = true;
      break;
    case '?':
      fprintf(stderr, "Błąd! Nieznana flaga lub brak argumentu do opcji!\n");
      return -1;
    }
  }
  return 0;
}