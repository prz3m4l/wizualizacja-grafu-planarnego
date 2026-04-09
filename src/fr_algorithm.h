#ifndef FR_ALGORITHM_H
#define FR_ALGORITHM_H

/* Minimalna odległość między wierzchołkami — ochrona przed dzieleniem przez 0 */
#define MIN_DIST 0.01

/* Współczynnik chłodzenia temperatury w algorytmie Fruchtermana-Reingolda */
#define TEMP_FACTOR 0.95

#include "graph.h"

/* Algorytm Fruchtermana-Reingolda do rozmieszczania wierzchołków grafu.
 * Opiera się na symulacji sił fizycznych między wierzchołkami. */
void fruchtermanReingold(Graph *g, int iterations, double width, double height);

#endif