#ifndef GRAPH_H
#define GRAPH_H

typedef struct Node {
  int *neighbours;
  int count; // ilosc sasiadow
  int size;
  int v;
  double x, y;
  double dx, dy; // sily
} Node;

void addVertex(Node *adjList, int v, int u);
#endif
