#include "algorithms.h"
#include "graph.h"
#include "io_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int opt;
  int width = 1000;
  int height = 1000;
  int iter = 100;
  char *input_file = NULL;
  char *output_file = NULL;
  char *algorithm_name = NULL;
  while ((opt = getopt(argc, argv, "i:o:w:h:t:a:")) != -1){
      switch(opt){
          case 'i':
            input_file = optarg;
            break;
          case 'o':
            output_file = optarg;
            break;
          case 'w':
            width=atoi(optarg);
            break;
          case 'h':
            height=atoi(optarg);
            break;
          case 't':
            iter=atoi(optarg);
            break;
          case 'a':
            algorithm_name = optarg;
            break;
          case '?':
            fprintf(stderr, "Błąd! Brak wszystkich wymaganych flag programu!\n");
            return -1;
      }
  }

  if(input_file == NULL || output_file == NULL){
      fprintf(stderr, "Błąd wczytania plików z argumentów wywołania!\n");
      return -1;
  }

  FILE *in_file = fopen(input_file, "r");
  if (in_file == NULL) {
    fprintf(stderr, "Blad wczytywania pliku\n");
    return -1;
  }

  /* wczytywanie grafu */
  Graph g = {0}; /* zabezpiecznie przed segmentation fault w cleanupOnError */
  if (loadGraph(in_file, &g, width, height) != 0) {
    fprintf(stderr, "Blad wczytywania grafu\n");
    fclose(in_file);
    return -1;
  }

  fclose(in_file);
  if(algorithm_name == NULL || strcmp(algorithm_name, "fruchterman")==0){
    fruchterman_reingold(&g, iter, width, height);
  }else {
    fprintf(stderr, "Błąd! Wprowadzono nieprawdiłową nazwę algorytmu!\n");
    freeGraph(&g);
    return -1;
  }
  FILE *out_file = fopen(output_file, "w");
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
