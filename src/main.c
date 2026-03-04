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
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    fprintf(stderr, "Blad wczytywania pliku");
    return -1;
  }

  /* wczytywanie grafu */
  Graph g = {0}; /* zabezpiecznie przed segmentation fault w cleanupOnError */
  if (loadGraph(file, &g, WIDTH, HEIGHT) != 0) {
    fprintf(stderr, "Blad wczytywania grafu\n");
    fclose(file);
    return -1;
  }

  fclose(file);

  fruchterman_reingold(&g, ITER, WIDTH, HEIGHT);

  for (int i = 0; i < (&g)->vertices_n; i++) {
    printf("%d %g %g\n", (&g)->vertices[i].v, (&g)->vertices[i].x,
           (&g)->vertices[i].y);
  }
  freeGraph(&g); /* zwolnienie pamięci zaalokowanej na wierzchołki i sąsiadów */
  return 0;
}
