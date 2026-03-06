#include "algorithms.h"
#include "graph.h"
#include "io_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_WIDTH  100000
#define MAX_HEIGHT 100000
#define MAX_ITER   100000

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int opt;
  int width = 1000;
  int height = 1000;
  int iter = 100;
  char *input_file = NULL;
  char *output_file = NULL;
  char *algorithm_name = NULL;
  bool isBinary = false;
  char *endptr = NULL;
  long parsedValue = 0;

  while ((opt = getopt(argc, argv, "i:o:w:h:t:a:b")) != -1){
      switch(opt){
          case 'i':
            input_file = optarg;
            break;
          case 'o':
            output_file = optarg;
            break;
          case 'w':
            errno = 0;
            parsedValue = strtol(optarg, &endptr, 10);
            if (errno != 0 || *endptr != '\0') {
                fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -w: %s\n", optarg);
                return -1;
            }
            width = (int)parsedValue;
            break;
          case 'h':
            errno = 0;
            parsedValue = strtol(optarg, &endptr, 10);
            if (errno != 0 || *endptr != '\0') {
                fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -h: %s\n", optarg);
                return -1;
            }
            height = (int)parsedValue;
            break;
          case 't':
            errno = 0;
            parsedValue = strtol(optarg, &endptr, 10);
            if (errno != 0 || *endptr != '\0') {
              fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -t: %s\n", optarg);
              return -1;
            }
            break;
          case 'a':
            algorithm_name = optarg;
            break;
          case 'b':
            isBinary = true;
            break;
          case '?':
            fprintf(stderr, "Błąd! Nieznana flaga lub brak argumentu do opcji!\n");
            return -1;
      }
  }

  if (width <= 0 || width > MAX_WIDTH || height <= 0 || height > MAX_HEIGHT || iter <= 0 || iter > MAX_ITER) {
    fprintf(stderr, "Błąd! Wartości width/height muszą być w zakresie [1, 100000], iter w zakresie [1, 100000]!\n");
    return -1;
}

  if(input_file == NULL || output_file == NULL){
      fprintf(stderr, "Błąd! Niepoprawne wczytanie plików z argumentów wywołania!\n");
      return -1;
  }

  FILE *in_file = fopen(input_file, "r");
  if (in_file == NULL) {
    fprintf(stderr, "Blad wczytywania pliku\n");
    return -1;
  }

  /* wczytywanie grafu */
  Graph graph = {0}; /* zabezpiecznie przed segmentation fault w cleanupOnError */
  if (loadGraph(in_file, &graph, width, height) != 0) {
    fprintf(stderr, "Blad wczytywania grafu\n");
    fclose(in_file);
    return -1;
  }

  fclose(in_file);
  if(algorithm_name == NULL || (strcmp(algorithm_name, "fruchterman")==0)){
    fruchterman_reingold(&graph, iter, width, height);
  }else {
    fprintf(stderr, "Błąd! Wprowadzono nieprawdiłową nazwę algorytmu!\n");
    freeGraph(&graph);
    return -1;
  }

  FILE *out_file = NULL;
  if(isBinary == false){
    out_file = fopen(output_file, "w");
  }else {
    out_file = fopen(output_file, "wb");
  }
  if (out_file == NULL) {
    fprintf(stderr, "Blad wczytywania pliku do zapisu!\n");
    freeGraph(&graph); /* zwolnienie pamięci przed wyrzuceniem błędu */
    return -1;
  }

  saveResults(out_file, &graph, isBinary);
  fclose(out_file);

  freeGraph(&graph); /* zwolnienie pamięci zaalokowanej na wierzchołki i sąsiadów */
  return 0;
}
