#ifndef ALGORITHMS_H
#define ALGORITHMS_H

/* Minimalna odległość między wierzchołkami — ochrona przed dzieleniem przez 0 */
#define MIN_DIST 0.01

/* Współczynnik chłodzenia temperatury w algorytmie Fruchtermana-Reingolda */
#define TEMP_FACTOR 0.95

/* Globalna stała sprężystości w algorytmie Kamady-Kawai */
#define K 1.0

#include "io_manager.h"
#include <math.h>
#include <stdlib.h>

/* Parametry sprężyny między parą wierzchołków (algorytm Kamady-Kawai) */
typedef struct {
    double length;    /* idealna długość sprężyny (l_ij) */
    double stiffness; /* stała sprężystości (k_ij) */
} Spring;

/* Algorytm Fruchtermana-Reingolda do rysowania grafów planarnych */
void fruchtermanReingold(Graph *g, int iterations, double width, double height);

/* Obliczanie macierzy najkrótszych odległości między wszystkimi parami wierzchołków (BFS) */
int **calculateDistances(Graph *graph);

/* Algorytm Kamady-Kawai do rysowania grafów planarnych */
void kamadaKawaiLayout(Graph *graph, int width, int height, int iterations);

#endif