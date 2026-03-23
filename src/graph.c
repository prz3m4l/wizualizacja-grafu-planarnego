#include "graph.h"

int addVertex(Node *vertices, int v, int u) {
  Node *vertex = &vertices[v];
  if (vertex->size == 0) {
    vertex->size = 2;
    vertex->neighbours = malloc(vertex->size * sizeof(int));
    if (vertex->neighbours == NULL) {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla relacji wierzchołka!\n");
      return -1;
    }
    vertex->count = 0;
  } else if (vertex->size <= vertex->count) {
    int newSize = vertex->size * 2;
    int *tmp = realloc(vertex->neighbours, newSize * sizeof(int));
    if (tmp == NULL) {
      fprintf(stderr,
              "Błąd! Nie można zaalokować pamięci dla sąsiadów wierzchołka!\n");
      return -1;
    } 
    vertex->neighbours = tmp;
        vertex->size = newSize; // Dopiero po sukcesie aktualizujemy rozmiar
    }
    vertex->neighbours[vertex->count++] = u;
    return 0;
}

void freeGraph(Graph *graph) {
  if (!graph)
    return;

  if (graph->vertices) {
    for (int i = 0; i < graph->vertices_n; ++i) {
      free(graph->vertices[i].name);
      graph->vertices[i].name = NULL;
      free(graph->vertices[i].neighbours);
      graph->vertices[i].neighbours = NULL;
    }
    free(graph->vertices);
  }

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
  graph->x=NULL;
  graph->y=NULL;
  graph->dx=NULL;
  graph->dy=NULL;
}

static void dfsVisit(const Graph *graph, int current_vertex, bool *visited) {
  visited[current_vertex] = true;
  for (int i = 0; i < (graph->vertices[current_vertex].count); i++) {
    int neighbor = graph->vertices[current_vertex].neighbours[i];
    if (visited[neighbor] == false) {
      dfsVisit(graph, neighbor, visited);
    }
  }
}

static int addExtraEdge(Graph *graph, int v, int u) {
  if (addVertex(graph->vertices, v, u) == -1)
    return -1;
  if (addVertex(graph->vertices, u, v) == -1)
    return -1;

  Edge *tmp = realloc(graph->edges, (graph->edges_n + 1) * sizeof(Edge));
  if (tmp == NULL) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla dodatkowej krawędzi!\n");
    return -1;
  }
  graph->edges = tmp;

  graph->edges[graph->edges_n].idA = v;
  graph->edges[graph->edges_n].idB = u;
  graph->edges[graph->edges_n].name = strdup("Auto-Generated-Edge");

  graph->edges_n++;
  return 0;
}

int ensureConnectivity(Graph *graph) {
  bool repaired = false;
  if (graph->vertices_n == 0)
    return 1;

  bool *visited = calloc(graph->vertices_n, sizeof(bool));
  if (visited == NULL) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla visited!\n");
    return -1;
  }

  dfsVisit(graph, 0, visited);
  for (int i = 0; i < graph->vertices_n; i++) {
    if (visited[i] == false) {
      repaired = true;
      if (addExtraEdge(graph, 0, i) == -1) {
        free(visited);
        return -1;
      }
      dfsVisit(graph, i, visited);
    }
  }
  free(visited);
  return repaired ? 0 : 1;
}