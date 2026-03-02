#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <stdio.h>
#include "graph.h"  /* dla struktury Node */

/*  Struktura Graf używana przez procedury wejścia/wyjścia. 
    Zawiera reprezentację w postaci listy sąsiedztwa wraz z metadanymi 
    o liczbie wierzchołków i krawędzi. */
struct Graph {
    Node *vertices;   /* dynamiczna tablica węzłów listy sąsiedztwa */
    int vertices_n;   /* liczba wierzchołków */
    int edges_n;      /* liczba krawędzi wczytanych z wejścia */
};

/* Wczytuje dane o grafie z pliku i inicjalizuje strukturę Graf */
int loadGraph(FILE *inputFiles, struct Graph *graf);

/* Zwalnia pamięć dynamiczną przydzieloną przez funkcję wczytajGraf */
void freeGraph(struct Graph *graf);

/* Zapisuje wyniki algorytmów do pliku wyjściowego */
void saveResults(FILE *outputFiles, struct Graph *graf);

#endif