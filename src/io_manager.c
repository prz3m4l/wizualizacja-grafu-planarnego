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
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci przy tworzeniu listy wierzchołków!\n");
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
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas rozszerzania tablicy krawędzi!\n");
      return -1;
    }
    list->names = tmp;
    list->capacity = newCapacity;
  }

  list->names[list->count] = strdup(name);
  return list->count++;
}

void cleanupVertexList(VertexList *list) {
  for (int i = 0; i < list->count; i++)
    free(list->names[i]);
  free(list->names);
}

static int readEdges(FILE *inputFile, Edge **edges_out, int *count_out, VertexList *vList){
  char buff[4096];
  int capacity = 16;
  int count = 0;
  Edge *edges = malloc(capacity * sizeof(Edge));
  if (!edges) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla tablicy krawędzi!\n");
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
    edges[count].weight = weight;
    count++;
  }

  if (count == 0) {
    fprintf(stderr, "Błąd! Liczba wczytanych krawędzi jest równa 0!\n");
    free(edges);
    return -1;
  }

  *edges_out = edges;
  *count_out = count;
  return 0;
}

static int allocateGraph(Graph *graph, int vCount, int eCount) {
  graph->vertices_n = vCount;
  graph->edges_n = eCount;
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
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla współrzędnych i wierzchołków!\n");
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
    addVertex(graph->vertices, graph->edges[i].idA, graph->edges[i].idB);
    addVertex(graph->vertices, graph->edges[i].idB, graph->edges[i].idA);
  }

  for (int i = 0; i < graph->vertices_n; i++) {
    graph->vertices[i].v = i;
    graph->x[i] = (double)rand() / RAND_MAX * width;
    graph->y[i] = (double)rand() / RAND_MAX * height;
    graph->vertices[i].name = strdup(vList.names[i]);
  }

  cleanupVertexList(&vList);
  return 0;
}

void saveResults(FILE *outputFile, Graph *graph, bool isBinary) {
  if (isBinary == false) {
    for (int i = 0; i < graph->vertices_n; i++) {
        fprintf(outputFile, "%s %g %g\n", graph->vertices[i].name, graph->x[i],
                graph->y[i]);
    }
  } else {
    for (int i = 0; i < graph->vertices_n; i++) {
      uint32_t name_len = strlen(graph->vertices[i].name);
      uint32_t len_be = toBigEndianUint32(name_len);
      double x_be = toBigEndianDouble(graph->x[i]);
      double y_be = toBigEndianDouble(graph->y[i]);
      
      fwrite(&len_be, sizeof(uint32_t), 1, outputFile);
      fwrite(graph->vertices[i].name, sizeof(char), name_len, outputFile);
      fwrite(&x_be, sizeof(double), 1, outputFile);
      fwrite(&y_be, sizeof(double), 1, outputFile);
    }
  }
}