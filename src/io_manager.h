#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "graph.h"
#include <math.h>
#include <stdio.h>

/* Zapisuje wyniki algorytmów do pliku wyjściowego */
void saveResults(FILE *outputFiles, Graph *graf);

/* Wczytuje dane o grafie z pliku i inicjalizuje strukturę Graf */
int loadGraph(FILE *inputFiles, Graph *graf, int width, int height);

/* Zwalnia pamięć dynamiczną przydzieloną przez funkcję wczytajGraf */
void freeGraph(Graph *graf);

#endif
