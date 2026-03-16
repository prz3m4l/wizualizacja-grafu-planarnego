#include "graph.h"
#include <stdbool.h>

int addVertex(Node *vertices, int v, int u) {
  Node *vertex = &vertices[v];
  vertex->v = v;
  if (vertex->size == 0) {
    vertex->size = 2;
    vertex->neighbours = malloc(vertex->size * sizeof(int));
    vertex->count = 0;
  } else if (vertex->size <= vertex->count) {
    vertex->size *= 2;
    int *tmp = realloc(vertex->neighbours, vertex->size * sizeof(int));
    if (tmp == NULL) {
      fprintf(stderr,
              "Błąd! Nie można zaalokować pamięci dla sąsiadów wierzchołka!\n");
      return -1;
    } else {
      vertex->neighbours = tmp;
    }
  }
  vertex->neighbours[vertex->count] = u;
  vertex->count++;
  return 0;
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

static bool dfsFindCycle(const Graph *graph, int current_vertex,
                         int parent_vertex, int *parents, int *cycles,
                         int *cycle_lenght, bool *visited) {
  visited[current_vertex] = true;
  parents[current_vertex] = parent_vertex;

  for (int i = 0; i < (graph->vertices[current_vertex].count); i++) {
    int neighbor = graph->vertices[current_vertex].neighbours[i];
    if (visited[neighbor] == false) {
      if (dfsFindCycle(graph, neighbor, parent_vertex, parents, cycles,
                       cycle_lenght, visited)) {
        return true;
      }

    } else if (neighbor != parent_vertex) { // Jeśli sąsiad jest odwiedzony i
                                            // nie jest rodzicem mamy cykl

      *cycle_lenght = 0;
      int curr = current_vertex;

      while (curr != neighbor && curr != 1) {
        cycles[*cycle_lenght++] = curr;
        curr = parents[curr];
      }
      cycles[*cycle_lenght++] = neighbor;

      return true;
    }
  }
  return false;
}

static int addExtraEdge(Graph *graph, int v, int u) {
  /* Aktualizacja listy sąsiedztwa */
  if (addVertex(graph->vertices, v, u) == -1) {
    return -1;
  }
  if (addVertex(graph->vertices, u, v) == -1) {
    return -1;
  }

  /* Zwiększenie tablicy krawędzi dla algorytmu rysującego */
  Edge *tmp = realloc(graph->edges, (graph->edges_n + 1) * sizeof(Edge));
  if (tmp == NULL) {
    return -1;
  }
  graph->edges = tmp;

  /* Uzupełnienie danych nowej krawędzi */
  graph->edges[graph->edges_n].idA = v;
  graph->edges[graph->edges_n].idB = u;
  graph->edges[graph->edges_n].weight = 1.0;
  graph->edges[graph->edges_n].name = strdup("Auto-Generated-Edge");

  graph->edges_n++;
  return 0;
}

int ensureConnectivity(Graph *graph) {
  bool repaired = false;
  if (graph->vertices_n == 0) {
    return 1;
  }
  bool *visited = calloc(graph->vertices_n, sizeof(bool));
  if (visited == NULL) {
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
  if (repaired) {
    return 0;
  }
  return 1;
}
