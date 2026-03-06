#include "graph.h"

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
    if(tmp == NULL){
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla sąsiadów wierzchołka!\n");
      return -1;
    }else {
        vertex->neighbours=tmp;
    }
  }
  vertex->neighbours[vertex->count] = u;
  vertex->count++;
  return 0;
}
