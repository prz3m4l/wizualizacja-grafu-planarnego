#include "algorithms.h"
#include "graph.h"
#include "io_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HEIGHT 1000
#define WIDTH 1000
#define ITER 100

int main(int argc, char *argv[]) {
  srand(time(NULL));
  if (argc < 2) {
    fprintf(stderr, "Nie udalo sie wczytac pliku!");
    return -1;
  }
  FILE *in_file = fopen(argv[1], "r");
  if (in_file == NULL) {
    fprintf(stderr, "Blad wczytywania pliku");
    return -1;
  }

  /* wczytywanie grafu */
  Graph g = {0}; /* zabezpiecznie przed segmentation fault w cleanupOnError */
  if (loadGraph(in_file, &g, WIDTH, HEIGHT) != 0) {
    fprintf(stderr, "Blad wczytywania grafu\n");
    fclose(in_file);
    return -1;
  }

  fclose(in_file);

  fruchterman_reingold(&g, ITER, WIDTH, HEIGHT);

  if (argc < 3) {
    fprintf(stderr, "Nie udalo sie wczytac pliku do zapisu!\n");
    freeGraph(&g); /* zwolnienie pamięci przed wyrzuceniem błędu */
    return -1;
  }
  FILE *out_file = fopen(argv[2], "w");
  if (out_file == NULL) {
    fprintf(stderr, "Blad wczytywania pliku do zapisu!\n");
    freeGraph(&g); /* zwolnienie pamięci przed wyrzuceniem błędu */
    return -1;
  }
  saveResults(out_file, &g);
  fclose(out_file);

  freeGraph(&g); /* zwolnienie pamięci zaalokowanej na wierzchołki i sąsiadów */
  return 0;
}
