#include "io_manager.h"

int is_little_endian(void) {
  uint16_t test = 0x0001;
  uint8_t byte;
  memcpy(&byte, &test, 1);
  return byte == 0x01;
}

uint32_t to_big_endian_int32(uint32_t val) {
  if (!is_little_endian()) {
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

double to_big_endian_double(double val) {
  if (!is_little_endian()) {
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

void initList(VertexList *list) {
  list->count = 0;
  list->capacity = 10;
  list->names = malloc(list->capacity * sizeof(char *));
}

// Dodaje nazwę tylko jeśli jej jeszcze nie ma
int addUnique(VertexList *list, const char *name) {
  // Sprawdź czy już istnieje
  for (int i = 0; i < list->count; i++) {
    if (strcmp(list->names[i], name) == 0)
      return i;
  }

  // Jeśli brak miejsca powiększ tablicę
  if (list->count >= list->capacity) {
    list->capacity *= 2;
    list->names = realloc(list->names, list->capacity * sizeof(char *));
  }

  list->names[list->count] = strdup(name);
  int new_id = list->count;
  list->count++;
  return new_id;
}

void cleanupList(VertexList *list) {
  for (int i = 0; i < list->count; i++)
    free(list->names[i]);
  free(list->names);
}

static int read_edges(FILE *inputFile, Edge **edges_out, int *count_out, VertexList *vList){
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

    int idA = addUnique(vList, nameA);
    int idB = addUnique(vList, nameB);

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

static int allocate_graph(Graph *graph, int v_count, int e_count) {
  graph->vertices_n = v_count;
  graph->edges_n = e_count;
  
  graph->vertices = calloc(v_count, sizeof(Node));
  graph->x = malloc(v_count * sizeof(double));
  graph->y = malloc(v_count * sizeof(double));
  graph->dx = calloc(v_count, sizeof(double));
  graph->dy = calloc(v_count, sizeof(double));

  if (!graph->vertices || !graph->x || !graph->y || !graph->dx || !graph->dy) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla współrzędnych i wierzchołków!\n");
    freeGraph(graph);
    return -1;
  }
  return 0;
}

int loadGraph(FILE *inputFile, Graph *graph, int width, int height) {
  VertexList vList;
  initList(&vList);
  Edge *temp_edges = NULL;
  int edges_count = 0;

  if (read_edges(inputFile, &temp_edges, &edges_count, &vList) != 0) {
    cleanupList(&vList);
    return -1;
  }

  if (allocate_graph(graph, vList.count, edges_count) != 0) {
    for(int i = 0; i<edges_count; i++){
      free(temp_edges[i].name);
    }
    free(temp_edges);
    cleanupList(&vList);
    return -1;
  }
  graph->edges = temp_edges;
  for (int i = 0; i < edges_count; i++) {
    addVertex(graph->vertices, graph->edges[i].idA, graph->edges[i].idB);
    addVertex(graph->vertices, graph->edges[i].idB, graph->edges[i].idA);
  }

  for (int i = 0; i < graph->vertices_n; i++) {
    graph->vertices[i].v = i;
    graph->x[i] = (double)rand() / RAND_MAX * width;
    graph->y[i] = (double)rand() / RAND_MAX * height;
  }

  cleanupList(&vList);
  return 0;
}

void freeGraph(Graph *graph) {
  if (!graph || !graph->vertices)
    return;
  for (int i = 0; i < graph->vertices_n; ++i) {
    free(graph->vertices[i].neighbours);
    graph->vertices[i].neighbours = NULL;
  }
  free(graph->vertices);

  if (graph->edges) {
    for (int i = 0; i < graph->edges_n; i++)
      free(graph->edges[i].name);
    free(graph->edges);
  }
  graph->edges = NULL;
  graph->vertices = NULL;
  graph->vertices_n = graph->edges_n = 0;
  free(graph->x);
  free(graph->y);
  free(graph->dx);
  free(graph->dy);
}

void saveResults(FILE *outputFile, Graph *graph, bool isBinary) {
  if (isBinary == false) {
    for (int i = 0; i < graph->vertices_n; i++) {
        fprintf(outputFile, "%d %g %g\n", graph->vertices[i].v, graph->x[i],
                graph->y[i]);
    }
  } else {
    for (int i = 0; i < graph->vertices_n; i++) {
        uint32_t v_be = to_big_endian_int32(graph->vertices[i].v);
        double x_be = to_big_endian_double(graph->x[i]);
        double y_be = to_big_endian_double(graph->y[i]);
        fwrite(&v_be, sizeof(uint32_t), 1, outputFile);
        fwrite(&x_be, sizeof(double), 1, outputFile);
        fwrite(&y_be, sizeof(double), 1, outputFile);
    }
  }
}
