#include "io_manager.h"

int isLittleEndian(void) {
  uint16_t test = 0x0001;
  uint8_t byte;
  memcpy(&byte, &test, 1);
  return byte == 0x01;
}

uint32_t toBigEndianUint32(uint32_t val) {
  if (!isLittleEndian()) {
    return val;
  }
  uint8_t in[4] = {0};
  uint8_t out[4] = {0};
  uint32_t result = 0;
  memcpy(in, &val, 4);
  for (int i = 0; i < 4; i++) {
    out[i] = in[3 - i];
  }
  memcpy(&result, out, 4);
  return result;
}

double toBigEndianDouble(double val) {
  if (!isLittleEndian()) {
    return val;
  }
  uint8_t in[8] = {0};
  uint8_t out[8] = {0};
  double result = 0.0;
  memcpy(in, &val, 8);
  for (int i = 0; i < 8; i++) {
    out[i] = in[7 - i];
  }
  memcpy(&result, out, 8);
  return result;
}

typedef struct {
  char **names;
  int count;
  int capacity;
} VertexList;


int initVertexList(VertexList *list) {
  list->count = 0;
  list->capacity = 10;
  list->names = malloc(list->capacity * sizeof(char *));
  if (list->names == NULL) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia listy wierzchołków!\n");
    return -1;
  }
  return 0;
}

// Dodaje nazwę tylko jeśli jej jeszcze nie ma
int addUniqueVertex(VertexList *list, const char *name) {
  // Sprawdź czy już istnieje
  for (int i = 0; i < list->count; i++) {
    if (strcmp(list->names[i], name) == 0)
      return i;
  }

  // Jeśli brak miejsca powiększ tablicę
  if (list->count >= list->capacity) {
    int newCapacity = list->capacity * 2;
    char **tmp = realloc(list->names, newCapacity * sizeof(char *));
    if (tmp == NULL) {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas rozszerzania tablicy nazw wierzchołków!\n");
      return -1;
    }
    list->names = tmp;
    list->capacity = newCapacity;
  }

  list->names[list->count] = strdup(name);
  if (list->names[list->count] == NULL) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas kopiowania nazwy wierzchołka!\n");
    return -1;
  }
  return list->count++;
}

void cleanupVertexList(VertexList *list) {
  for (int i = 0; i < list->count; i++)
    free(list->names[i]);
  free(list->names);
}

static int readEdges(FILE *inputFile, Edge **edgesOut, int *countOut, VertexList *vList){
  char buff[4096];
  int capacity = 16;
  int count = 0;
  Edge *edges = malloc(capacity * sizeof(Edge));
  if (!edges) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia tablicy krawędzi!\n");
    return -1;
  }

  char tempName[256] = {0}; 
  char nameA[256] = {0};
  char nameB[256] = {0};
  double weight = 0.0;

  while (fgets(buff, sizeof(buff), inputFile)) {
    if (buff[0] == '\n' || buff[0] == '#') continue;

    if(strchr(buff, '\n') == NULL && !feof(inputFile)){
      fprintf(stderr, "Błąd! Linia w pliku wejściowym jest zbyt długa!\n");
      for(int i = 0; i < count; i++) free(edges[i].name);
      free(edges);
      return -1;
    }

    if (count >= capacity) {
      capacity *= 2;
      Edge *tmp = realloc(edges, capacity * sizeof(Edge));
      if (!tmp) {
        fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas rozszerzania tablicy krawędzi!\n");
        for(int i = 0; i < count; i++) free(edges[i].name);
        free(edges);
        return -1;
      }
      edges = tmp;
    }

    if (sscanf(buff, "%255s %255s %255s %lf", tempName, nameA, nameB, &weight) != 4) {
      fprintf(stderr, "Błąd! Linia w pliku nie pasuje do formatu: <nazwa> <idA> <idB> <waga>!\n");
      for(int i = 0; i < count; i++) free(edges[i].name);
      free(edges);
      return -1;
    }

    if (!isfinite(weight) || weight < 0) {
      fprintf(stderr, "Błąd! Waga krawędzi musi być nieujemną liczbą skończoną!\n");
      for(int i = 0; i < count; i++) free(edges[i].name);
      free(edges);
      return -1;
    }

    int idA = addUniqueVertex(vList, nameA);
    if (idA == -1) {
        for (int i = 0; i < count; i++) free(edges[i].name);
        free(edges);
        return -1;
    }
    int idB = addUniqueVertex(vList, nameB);
    if (idB == -1) {
        for (int i = 0; i < count; i++) free(edges[i].name);
        free(edges);
        return -1;
    }

    if (idA == idB) {
      fprintf(stderr, "Ostrzeżenie: Wykryto pętlę własną dla wierzchołka %s\n", nameA);
    }

    edges[count].idA = idA;
    edges[count].idB = idB;
    edges[count].name = strdup(tempName);
    if (edges[count].name == NULL) {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas kopiowania nazwy krawędzi!\n");
      for(int i = 0; i < count; i++) free(edges[i].name);
      free(edges);
      return -1;
    }
    count++;
  }

  if (count == 0) {
    fprintf(stderr, "Błąd! Liczba wczytanych krawędzi jest równa 0!\n");
    free(edges);
    return -1;
  }

  *edgesOut = edges;
  *countOut = count;
  return 0;
}

static int allocateGraph(Graph *graph, int vCount, int eCount) {
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

  if (!graph->vertices || !graph->x || !graph->y || !graph->dx || !graph->dy) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas przydzielania współrzędnych i grafu!\n");
    freeGraph(graph);
    return -1;
  }
  return 0;
}

int loadGraph(FILE *inputFile, Graph *graph, int width, int height) {
  VertexList vList;
  if (initVertexList(&vList) != 0) {
    return -1; 
  }
  Edge *tempEdges = NULL;
  int edgesCount = 0;

  if (readEdges(inputFile, &tempEdges, &edgesCount, &vList) != 0) {
    cleanupVertexList(&vList);
    return -1;
  }

  if (vList.count == 0) {
    fprintf(stderr, "Błąd! Plik nie zawiera żadnych wierzchołków! \n");
    cleanupVertexList(&vList);
    free(tempEdges);
    return -1;
  }

  if (allocateGraph(graph, vList.count, edgesCount) != 0) {
    for(int i = 0; i<edgesCount; i++){
      free(tempEdges[i].name);
    }
    free(tempEdges);
    cleanupVertexList(&vList);
    return -1;
  }
  graph->edges = tempEdges;
  for (int i = 0; i < edgesCount; i++) {
    if (addVertex(graph->vertices, graph->edges[i].idA, graph->edges[i].idB) == -1 ||
        addVertex(graph->vertices, graph->edges[i].idB, graph->edges[i].idA) == -1) {
      cleanupVertexList(&vList);
      freeGraph(graph);
      return -1;
    }
  }

  for (int i = 0; i < graph->verticesCount; i++) {
    graph->x[i] = (double)rand() / RAND_MAX * width;
    graph->y[i] = (double)rand() / RAND_MAX * height;
    graph->vertices[i].name = strdup(vList.names[i]);
    if (graph->vertices[i].name == NULL) {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas kopiowania nazwy!\n");
      cleanupVertexList(&vList);
      freeGraph(graph);
      return -1;
    }
  }

  cleanupVertexList(&vList);
  return 0;
}

void saveResults(FILE *outputFile, Graph *graph, bool isBinary) {
  if (isBinary == false) {
    for (int i = 0; i < graph->verticesCount; i++) {
        fprintf(outputFile, "%s %g %g\n", graph->vertices[i].name, graph->x[i],
                graph->y[i]);
    }
  } else {
    for (int i = 0; i < graph->verticesCount; i++) {
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