#include "algorithms.h"
#include "graph.h"
#include "io_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  srand(time(NULL));
  if (argc < 2) {
    fprintf(stderr, "Nie udalo sie wczytac pliku!");
    return -1;
  }
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    fprintf(stderr, "Blad wczytywania pliku");
    return -1;
  }

  /* wczytywanie grafu */
  struct Graph graf = {
      0}; /* zabezpiecznie przed segmentation fault w cleanupOnError */
  if (loadGraph(file, &graf) != 0) {
    fprintf(stderr, "Blad wczytywania grafu\n");
    fclose(file);
    return -1;
  }

  fclose(file);
  freeGraph(
      &graf); /* zwolnienie pamięci zaalokowanej na wierzchołki i sąsiadów */
  return 0;
}
