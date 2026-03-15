#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#define MIN_DIST 0.01
#define TEMP_FACTOR 0.95

/* Globalna stała sprężystości */
#define K 1.0

#include "io_manager.h"
#include <math.h>
#include <stdlib.h>

typedef struct {
    double l;
    double k;
} Spring;

/* Algorytm Fruchtermana-Reingolda do rysowania grafów planarnych */
void fruchterman_reingold(Graph *g, int iterations, double width,
                          double height);

/* Obliczanie odległości między wierzchołkami */
int **calculate_distances(Graph *graph);

/* Algorytm Kamady-Kawai do rysowania grafów planarnych */
void kamada_kawai_layout(Graph *graph, int width, int height, int iterations);

#endif
