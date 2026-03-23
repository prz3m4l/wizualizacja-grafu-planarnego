#ifndef KK_ALGORITHM_H
#define KK_ALGORITHM_H

/* Globalna stała sprężystości w algorytmie Kamady-Kawai */
#define K 1.0

#include "graph.h"

/* Algorytm Kamady-Kawai do rysowania grafów planarnych */
void kamadaKawaiLayout(Graph *graph, int width, int height, int iterations);

#endif