#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Węzeł listy sąsiedztwa reprezentujący jeden wierzchołek grafu */
typedef struct Node {
  int *neighbours; /* dynamiczna tablica identyfikatorów sąsiadów */
  int count;       /* aktualna liczba sąsiadów */
  int size;        /* pojemność tablicy neighbours */
  char *name;      /* nazwa wierzchołka */
} Node;

/* Krawędź grafu przechowująca dane wczytane z pliku wejściowego */
typedef struct Edge {
  char   *name;   /* nazwa krawędzi */
  int     idA;    /* identyfikator wierzchołka A */
  int     idB;    /* identyfikator wierzchołka B */
  double  weight; /* waga krawędzi */
} Edge;

/* Główna struktura grafu używana przez wszystkie moduły programu.
   Zawiera reprezentację w postaci listy sąsiedztwa oraz listy krawędzi
   wraz z metadanymi i tablicami współrzędnych. */
typedef struct Graph {
  Edge   *edges;      /* tablica krawędzi wczytanych z pliku */
  Node   *vertices;   /* dynamiczna tablica węzłów listy sąsiedztwa */
  int     verticesCount; /* liczba wierzchołków */
  int     edgesCount;    /* liczba krawędzi */
  double *x;          /* współrzędne x wierzchołków */
  double *y;          /* współrzędne y wierzchołków */
  double *dx;         /* składowe x sił działających na wierzchołki */
  double *dy;         /* składowe y sił działających na wierzchołki */
} Graph;


/* Zwalnia dynamicznie zaalokowaną pamięć dla całej struktury grafu,
   w tym tablic krawędzi, wierzchołków, sąsiadów, współrzędnych i nazw. */
void freeGraph(Graph *graph);

/* Dodaje wierzchołek u do listy sąsiedztwa wierzchołka v.
   Zwraca 0 przy sukcesie, -1 przy błędzie alokacji pamięci. */
int addVertex(Node *adjList, int v, int u);

/* Sprawdza spójność grafu i w razie potrzeby dodaje brakujące krawędzie.
   Zwraca 1 jeśli graf był spójny, 0 jeśli dokonano naprawy,
   -1 przy błędzie alokacji pamięci. */
int ensureConnectivity(Graph *graph);

#endif
