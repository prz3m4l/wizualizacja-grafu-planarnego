#include "io_manager.h"

static void cleanupOnError(Graph *graph, Edge *edges) {
  if (edges)
    free(edges);
  if (graph && graph->vertices) {
    for (int i = 0; i < graph->vertices_n; ++i) {
      free(graph->vertices[i].neighbours);
    }
    free(graph->vertices);
    graph->vertices = NULL;
  }
}

int loadGraph(FILE *inputFile, Graph *graph, int width, int height) {
  if (!inputFile || !graph) {
    return -1;
  }

  char buff[4096];
  int edges_capacity = 16;
  int edges_count = 0;
  int max_v = 0;

  Edge *edges = malloc(edges_capacity * sizeof(Edge));
  if (!edges) {
    return -1;
  }
  while (fgets(buff, sizeof(buff), inputFile)) {
    if (buff[0] == '\n' || buff[0] == '#')
      continue; /* uznajemy puste/komentarz za pomijalne */
    if (edges_count >= edges_capacity) {
      edges_capacity *= 2;
      Edge *tmp = realloc(edges, edges_capacity * sizeof(Edge));
      if (!tmp) {
        cleanupOnError(graph, edges);
        return -1;
      }
      edges = tmp;
    }
    if (sscanf(buff, "%9s %d %d %lf", edges[edges_count].name,
               &edges[edges_count].idA, &edges[edges_count].idB,
               &edges[edges_count].weight) != 4) {
      cleanupOnError(graph, edges);
      return -1;
    }

    /* sprawdzanie identyfikatorów nieujemnych oraz czy waga jest skończona */
    if (edges[edges_count].idA < 0 || edges[edges_count].idB < 0 ||
        !isfinite(edges[edges_count].weight)) {
      cleanupOnError(graph, edges);
      return -1;
    }

    if (edges[edges_count].idA > max_v)
      max_v = edges[edges_count].idA;
    if (edges[edges_count].idB > max_v)
      max_v = edges[edges_count].idB;
    edges_count++;
  }

  if (edges_count == 0) {
    cleanupOnError(graph, edges);
    return -1;
  }

  graph->edges_n = edges_count;
  graph->vertices_n = max_v + 1;
  graph->vertices = calloc(graph->vertices_n, sizeof(Node));
  if (!graph->vertices) {
    cleanupOnError(graph, edges);
    return -1;
  }

  for (int i = 0; i < edges_count; i++) {
    addVertex(graph->vertices, edges[i].idA, edges[i].idB);
    addVertex(graph->vertices, edges[i].idB, edges[i].idA);
  }

  for (int i = 0; i < graph->vertices_n; i++) {
    graph->vertices[i].x = (double)rand() / RAND_MAX * width;
    graph->vertices[i].y = (double)rand() / RAND_MAX * height;
  }
  graph->edges = edges;
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
    free(graph->edges);
  }
  graph->edges = NULL;
  graph->vertices = NULL;
  graph->vertices_n = graph->edges_n = 0;
}

void saveResults(FILE *outputFile, Graph *graph) {
  for (int i = 0; i < graph->vertices_n; i++) {
    fprintf(outputFile, "%d %g %g\n", graph->vertices[i].v,
            graph->vertices[i].x, graph->vertices[i].y);
  }
}
