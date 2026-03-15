#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "graph.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Zapisuje wyniki algorytmów do pliku wyjściowego */
void saveResults(FILE *outputFiles, Graph *graf, bool isBinary);

/* Wczytuje dane o grafie z pliku i inicjalizuje strukturę Graf */
int loadGraph(FILE *inputFiles, Graph *graf, int width, int height);

/* Zwalnia pamięć dynamiczną przydzieloną przez funkcję wczytajGraf */
void freeGraph(Graph *graf);

/* Sprawdza czy procesor jest Little-Endian */
int is_little_endian(void);

/* Zamienia bajty w int na Big-Endian, kiedy to potrzebne */
uint32_t to_big_endian_int32(uint32_t val);

/* Zamienia bajty w double na Big-Endian, kiedy to potrzebne */
double to_big_endian_double(double val);

#endif
