#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Węzeł listy sasiedztwa
typedef struct Node {
  int *neighbours;
  int count; // ilosc sasiadow
  int size;
  int v;
} Node;

/*  Struktura Edge przechowująca dane wczytane z pliku na temat połączeń między
 * wierzchołkami */
typedef struct Edge {
  char *name;
  int idA;
  int idB;
  double weight;
} Edge;

/*  Struktura Graf używana przez procedury wejścia/wyjścia.
    Zawiera reprezentację w postaci listy sąsiedztwa oraz listy krawędzi wraz z
   metadanymi o liczbie wierzchołków i krawędzi. */
typedef struct Graph {
  Edge *edges;
  Node *vertices; /* dynamiczna tablica węzłów listy sąsiedztwa */
  int vertices_n; /* liczba wierzchołków */
  int edges_n;    /* liczba krawędzi wczytanych z wejścia */
  // Pozycja
  double *x;
  double *y;
  // Sily
  double *dx;
  double *dy;
} Graph;

/* Dodaje wierzchołek do listy sąsiedztwa */
int addVertex(Node *adjList, int v, int u);

/* Sprawdzanie spójność grafu */
int ensureConnectivity(Graph *graph);

bool isGraphPlanar(Graph *graph);

int makeGraphPlanar(Graph *graph);

#endif
