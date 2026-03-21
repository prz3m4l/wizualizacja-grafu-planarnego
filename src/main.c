#include "algorithms.h"
#include "graph.h"
#include "io_manager.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_WIDTH 100000
#define MAX_HEIGHT 100000
#define MAX_ITER 100000

int main(int argc, char *argv[]) {
  int opt = 0;
  int width = 1000;
  int height = 1000;
  int iter = 100;
  bool isBinary = false;
  bool isText = false;
  char *inputFile = NULL;
  char *outputFile = NULL;
  char *algorithmName = NULL;
  char *endPtr = NULL;
  int seed = 0;
  long parsedValue = 0;

  while ((opt = getopt(argc, argv, "i:o:w:h:t:a:b:s:")) != -1) {
    switch (opt) {
    case 'i':
      inputFile = optarg;
      break;
    case 'o':
      isText = true;
      outputFile = optarg;
      break;
    case 'w':
      errno = 0;
      parsedValue = strtol(optarg, &endPtr, 10);
      if (errno != 0 || *endPtr != '\0') {
        fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -w: %s!\n", optarg);
        return -1;
      }
      width = (int)parsedValue;
      break;
    case 'h':
      errno = 0;
      parsedValue = strtol(optarg, &endPtr, 10);
      if (errno != 0 || *endPtr != '\0') {
        fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -h: %s!\n", optarg);
        return -1;
      }
      height = (int)parsedValue;
      break;
    case 't':
      errno = 0;
      parsedValue = strtol(optarg, &endPtr, 10);
      if (errno != 0 || *endPtr != '\0') {
        fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -t: %s!\n", optarg);
        return -1;
      }
      iter = (int)parsedValue;
      break;
    case 'a':
      algorithmName = optarg;
      break;
    case 'b':
      isBinary = true;
      outputFile = optarg;
      break;
    case 's':
      errno = 0;
      parsedValue = strtol(optarg, &endPtr, 10);
      if (errno != 0 || *endPtr != '\0') {
        fprintf(stderr, "Błąd! Nieprawidłowa wartość dla -s: %s!\n", optarg);
        return -1;
      }
      seed = (int)parsedValue;
      break;
    case '?':
      fprintf(stderr, "Błąd! Nieznana flaga lub brak argumentu do opcji!\n");
      return -1;
    }
  }

  if (seed == 0) {
    srand(time(NULL));
  } else {
    srand(seed);
  }

  if (width <= 0 || width > MAX_WIDTH || height <= 0 || height > MAX_HEIGHT ||
      iter <= 0 || iter > MAX_ITER) {
    fprintf(stderr, "Błąd! Wartości width/height muszą być w zakresie [1, "
                    "100000], iter w zakresie [1, 100000]!\n");
    return -1;
  }

  if (inputFile == NULL || outputFile == NULL) {
    fprintf(stderr, "Błąd! Nie podano pliku wejściowego lub wyjściowego!\n");
    return -1;
  }

  if (isText == true && isBinary == true) {
    fprintf(stderr, "Błąd! Wybrano jednocześnie zapis wyników w formie "
                    "tekstowej i binarnej!");
    return -1;
  }

  FILE *inFile = fopen(inputFile, "r");
  if (inFile == NULL) {
    fprintf(stderr, "Błąd! Nie można otworzyć pliku wejściowego!\n");
    return -1;
  }

  /* wczytywanie grafu */
  Graph graph = {
      0}; /* zabezpiecznie przed segmentation fault w cleanupOnError */
  if (loadGraph(inFile, &graph, width, height) != 0) {
    fprintf(stderr, "Błąd! Nie można wczytać grafu z pliku!\n");
    fclose(inFile);
    return -1;
  }

  int removed = makeGraphPlanar(&graph);
  if (removed == -1) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas naprawy planarności!\n");
    freeGraph(&graph);
    fclose(in_file);
    return -1;
  }

  int connected = ensureConnectivity(&graph);
  if(connected == -1){
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci dla tablicy odwiedzonych wierzchołków!\n");
    fclose(inFile);
    freeGraph(&graph);
    return -1;
  }else if(connected == 0){
    fprintf(stderr, "Ostrzeżenie: Graf był niespójny! Automatycznie dodano brakujące krawędzie.\n");
  }
  fclose(in_file);
  
  if (algorithm_name == NULL || (strcmp(algorithm_name, "fruchterman") == 0)) {
    fruchterman_reingold(&graph, iter, width, height);
  } else if (strcmp(algorithm_name, "kamada") == 0) {
    kamada_kawai_layout(&graph, width, height, iter);
  } else {
    fprintf(stderr, "Błąd! Podana nazwa algorytmu jest nieprawidłowa!\n");
    freeGraph(&graph);
    return -1;
  }

  FILE *outFile = NULL;
  if (isBinary == false) {
    outFile = fopen(outputFile, "w");
  } else {
    outFile = fopen(outputFile, "wb");
  }
  if (outFile == NULL) {
    fprintf(stderr, "Błąd! Nie można otworzyć pliku wyjściowego do zapisu!\n");
    freeGraph(&graph); /* zwolnienie pamięci przed wyrzuceniem błędu */
    return -1;
  }

  saveResults(outFile, &graph, isBinary);
  fclose(outFile);

  freeGraph(&graph); /* zwolnienie pamięci zaalokowanej na wierzchołki i sąsiadów */
  return 0;
}
