#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "graph.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_WIDTH 100000
#define MAX_HEIGHT 100000
#define MAX_ITER 100000

typedef struct
{
  int width;
  int height;
  int iter;
  int seed;
  bool isBinary;
  bool isText;
  bool isSeedSet;
  char *inputFile;
  char *outputFile;
  char *algorithmName;
} CliFlags;

/* Analizuje flagi wiersza poleceń i zapisuje konfigurację w strukturze CliFlags.
 * Zwraca 0 po pomyślnym przetworzeniu, w przeciwnym razie -1. */
int parseCliFlags(int argc, char *argv[], CliFlags *flags);

/* Zapisuje wyniki algorytmów do pliku wyjściowego */
void saveResults(FILE *outputFiles, Graph *graf, bool isBinary);

/* Wczytuje dane o grafie z pliku i inicjalizuje strukturę Graf */
int loadGraph(FILE *inputFiles, Graph *graf, int width, int height);

/* Zwraca 1, jeśli architektura to Little-Endian, w przeciwnym razie 0 */
int isLittleEndian(void);

/* Konwertuje 32-bitową liczbę całkowitą z natywnego porządku bajtów na Big-Endian */
uint32_t toBigEndianUint32(uint32_t val);

/* Konwertuje liczbę zmiennoprzecinkową podwójnej precyzji z natywnego porządku bajtów na Big-Endian */
double toBigEndianDouble(double val);

#endif
