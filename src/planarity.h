#ifndef PLANARITY_H
#define PLANARITY_H

#include "graph.h"
#include <stdbool.h>

/* Sprawdza czy podany graf jest planarny (nie posiada przecinających się krawędzi).
 * Zwraca true jeśli graf jest planarny, w przeciwnym razie false. */
bool isGraphPlanar(Graph *graph);

/* Próbuje uczynić graf planarnym usuwając krawędzie psujące jego planarność.
 * Zwraca liczbę usuniętych krawędzi, 0 jeśli graf już był planarny,
 * lub -1 w przypadku błędu alokacji. */
int makeGraphPlanar(Graph *graph);

#endif