#ifndef KK_ALGORITHM_H
#define KK_ALGORITHM_H

/* Globalna stała sprężystości w algorytmie Kamady-Kawai */
#define K 1.0

#include "graph.h"

/* Algorytm Kamady-Kawai do rozmieszczania wierzchołków grafu.
 * Wykorzystuje model sprężyn, których optymalna długość jest proporcjonalna
 * do odległości grafowej (najkrótszej ścieżki) między wierzchołkami.
 * Przyjmuje parametry wymiarów obszaru rysowania oraz liczbę iteracji optymalizacji. */
void kamadaKawaiLayout(Graph *graph, int width, int height, int iterations);

#endif