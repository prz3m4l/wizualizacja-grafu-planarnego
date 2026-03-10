#include "io_manager.h"

int is_little_endian(void) {
  uint16_t test = 0x0001;
  uint8_t byte;
  memcpy(&byte, &test, 1);
  return byte == 0x01;
}

int32_t to_big_endian_int32(int32_t val){
  if (!is_little_endian()){
    return val;
  }
  uint8_t in[4]={0};
  uint8_t out[4]={0};
  int32_t result = 0;
  memcpy(in, &val, 4);
  for (int i = 0; i < 4; i++) {
        out[i] = in[3 - i];
  }
  memcpy(&result, out, 4);
  return result;
}

double to_big_endian_double(double val){
  if (!is_little_endian()){
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

static void cleanupOnError(Graph *graph, Edge *edges) {
  if (edges)
    free(edges);

  graph->edges = NULL; /* Dodanie czyszczenia graph->edges */

  if (graph && graph->vertices) {
    for (int i = 1; i < graph->vertices_n; ++i) {
      free(graph->vertices[i].neighbours);
    }
    free(graph->vertices);
    graph->vertices = NULL;
  }
  if (graph->x) {
    free(graph->x);
    graph->x = NULL;
  }
  if (graph->y) {
    free(graph->y);
    graph->y = NULL;
  }
  if (graph->dx) {
    free(graph->dx);
    graph->dx = NULL;
  }
  if (graph->dy) {
    free(graph->dy);
    graph->dy = NULL;
  }

  graph->vertices_n = 0;
  graph->edges_n = 0;
}

int loadGraph(FILE *inputFile, Graph *graph, int width, int height) {
  if (!inputFile || !graph) {
    fprintf(stderr, "Błąd! Przekazano nieprawidłowy wskaźnik pliku lub grafu!\n");
    return -1;
  }

  // Zabezpieczenie na wypadek smieci
  graph->vertices = NULL;
  graph->x = NULL;
  graph->y = NULL;
  graph->dx = NULL;
  graph->dy = NULL;
  graph->vertices_n = 0;
  graph->edges_n = 0;

  char buff[4096];
  int edges_capacity = 16;
  int edges_count = 0;
  int max_v = 0;
  Edge *edges = malloc(edges_capacity * sizeof(Edge));
  if (!edges) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla tablicy krawędzi!\n");
    return -1;
  }
  while (fgets(buff, sizeof(buff), inputFile)) {
    if (buff[0] == '\n' || buff[0] == '#')
      continue; /* uznajemy puste/komentarz za pomijalne */

    /* Sprawdzenie czy linia zmieściła się w buforze */
    if (strchr(buff, '\n') == NULL && !feof(inputFile)) {
        cleanupOnError(graph, edges);
        fprintf(stderr, "Błąd! Linia w pliku wejściowym jest zbyt długa!\n");
        return -1;
    }

    if (edges_count >= edges_capacity) {
      edges_capacity *= 2;
      Edge *tmp = realloc(edges, edges_capacity * sizeof(Edge));
      if (!tmp) {
        cleanupOnError(graph, edges);
        fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas rozszerzania tablicy krawędzi!\n");
        return -1;
      }
      edges = tmp;
    }

    if (sscanf(buff, "%9s %d %d %lf", edges[edges_count].name,
               &edges[edges_count].idA, &edges[edges_count].idB,
               &edges[edges_count].weight) != 4) {
      cleanupOnError(graph, edges);
      fprintf(stderr, "Błąd! Linia w pliku nie pasuje do formatu: <nazwa> <idA> <idB> <waga>!\n");
      return -1;
    }

    /* sprawdzanie identyfikatorów nieujemnych oraz czy waga jest skończona */
    if (edges[edges_count].idA < 0 || edges[edges_count].idB < 0){
      cleanupOnError(graph, edges);
      fprintf(stderr,
              "Błąd! Identyfikatory wierzchołków muszą być nieujemne!\n");
      return -1;
    }else if(!isfinite(edges[edges_count].weight) || edges[edges_count].weight < 0){
      cleanupOnError(graph, edges);
      fprintf(stderr,
              "Błąd! Waga krawędzi musi być nieujemną liczbą skończoną!\n");
      return -1;
    }

    if (edges[edges_count].idA == edges[edges_count].idB) {
      fprintf(stderr, "Ostrzeżenie: Wykryto pętlę własną dla wierzchołka %d\n", edges[edges_count].idA);
    }

    if (edges[edges_count].idA > max_v)
      max_v = edges[edges_count].idA;
    if (edges[edges_count].idB > max_v)
      max_v = edges[edges_count].idB;
    edges_count++;
  }

  if (edges_count == 0) {
    cleanupOnError(graph, edges);
    fprintf(stderr, "Błąd! Liczba wczytanych krawędzi jest równa 0!\n");
    return -1;
  }

  graph->edges_n = edges_count;
  graph->vertices_n = max_v + 1;
  // Alokacja pamieci
  graph->vertices = calloc(graph->vertices_n, sizeof(Node));
  graph->x = malloc(graph->vertices_n * sizeof(double));
  graph->y = malloc(graph->vertices_n * sizeof(double));
  graph->dx = calloc(graph->vertices_n, sizeof(double));
  graph->dy = calloc(graph->vertices_n, sizeof(double));

  if (!graph->vertices || !graph->x || !graph->y || !graph->dx || !graph->dy) {
    cleanupOnError(graph, edges);
    fprintf(stderr,
            "Błąd! Nie można zaalokować pamięci dla współrzędnych i wierzchołków!\n");
    return -1;
  }

  for (int i = 0; i < edges_count; i++) {
    addVertex(graph->vertices, edges[i].idA, edges[i].idB);
    addVertex(graph->vertices, edges[i].idB, edges[i].idA);
  }

  for (int i = 0; i < graph->vertices_n; i++) {
    graph->vertices[i].v = i;
    graph->x[i] = (double)rand() / RAND_MAX * width;
    graph->y[i] = (double)rand() / RAND_MAX * height;
  }
  graph->edges = edges;
  return 0;
}

void freeGraph(Graph *graph) {
  if (!graph || !graph->vertices)
    return;
  for (int i = 1; i < graph->vertices_n; ++i) {
    free(graph->vertices[i].neighbours);
    graph->vertices[i].neighbours = NULL;
  }
  free(graph->vertices);

  if (graph->edges) {
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
  if(isBinary == false){
    for (int i = 0; i < graph->vertices_n; i++) {
      if (graph->vertices[i].count > 0){
        fprintf(outputFile, "%d %g %g\n", graph->vertices[i].v, graph->x[i],
                graph->y[i]);
        }
    }
  }else {
    for (int i = 0; i < graph->vertices_n; i++) {
      if (graph->vertices[i].count > 0){
        int32_t v_be = to_big_endian_int32(graph->vertices[i].v);
        double x_be = to_big_endian_double(graph->x[i]);
        double y_be = to_big_endian_double(graph->y[i]);
        fwrite(&v_be, sizeof(int32_t), 1, outputFile);
        fwrite(&x_be, sizeof(double), 1, outputFile);
        fwrite(&y_be, sizeof(double), 1, outputFile);
      }
    }
  }
}
