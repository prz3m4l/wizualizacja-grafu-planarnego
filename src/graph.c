#include "graph.h"

void addVertex(Node *vertices, int v, int u) {
  Node *vertex = &vertices[v];
  vertex->v = v;
  if (vertex->size == 0) {
    vertex->size = 2;
    vertex->neighbours = malloc(vertex->size * sizeof(int));
    vertex->count = 0;
  } else if (vertex->size <= vertex->count) {
    vertex->size *= 2;
    vertex->neighbours =
        realloc(vertex->neighbours, vertex->size * sizeof(int));
  }
  vertex->neighbours[vertex->count] = u;
  vertex->count++;
}
