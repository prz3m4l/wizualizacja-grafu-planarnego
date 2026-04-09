#include "fr_algorithm.h"
#include "kk_algorithm.h"
#include "graph.h"
#include "planarity.h"
#include "io_manager.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Punkt wejścia do programu.
 * Inicjalizuje konfigurację z flag CLI, wczytuje graf, naprawia ewentualną niespójność
 * i nieplanarność, a następnie uruchamia wybrany algorytm i zapisuje wynik do pliku. */
int main(int argc, char *argv[])
{
  CliFlags flags;
  if (parseCliFlags(argc, argv, &flags) == -1)
  {
    return -1;
  }

  if (flags.isSeedSet)
  {
    srand(flags.seed);
  }
  else
  {
    srand(time(NULL));
  }

  if (flags.width <= 0 || flags.width > MAX_WIDTH || flags.height <= 0 || flags.height > MAX_HEIGHT ||
      flags.iter <= 0 || flags.iter > MAX_ITER)
  {
    fprintf(stderr, "Błąd! Wartości width/height muszą być w zakresie [1, "
                    "100000], iter w zakresie [1, 100000]!\n");
    return -1;
  }

  if (flags.inputFile == NULL || flags.outputFile == NULL)
  {
    fprintf(stderr, "Błąd! Nie podano pliku wejściowego lub wyjściowego!\n");
    return -1;
  }

  if (flags.isText == true && flags.isBinary == true)
  {
    fprintf(stderr, "Błąd! Wybrano jednocześnie zapis wyników w formie "
                    "tekstowej i binarnej!");
    return -1;
  }

  FILE *inFile = fopen(flags.inputFile, "r");
  if (inFile == NULL)
  {
    fprintf(stderr, "Błąd! Nie można otworzyć pliku wejściowego!\n");
    return -1;
  }

  /* wczytywanie grafu */
  Graph graph = {0};
  if (loadGraph(inFile, &graph, flags.width, flags.height) != 0)
  {
    fprintf(stderr, "Błąd! Nie można wczytać grafu z pliku!\n");
    fclose(inFile);
    return -1;
  }

  int removed = makeGraphPlanar(&graph);
  if (removed == -1)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas naprawy planarności!\n");
    freeGraph(&graph);
    fclose(inFile);
    return -1;
  }

  int connected = ensureConnectivity(&graph);
  if (connected == -1)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas inicjalizacji tablicy postępu (visited)!\n");
    fclose(inFile);
    freeGraph(&graph);
    return -1;
  }
  else if (connected == 0)
  {
    fprintf(stderr, "Ostrzeżenie: Graf był niespójny! Automatycznie dodano brakujące krawędzie.\n");
  }
  fclose(inFile);

  if (flags.algorithmName == NULL || (strcmp(flags.algorithmName, "fruchterman") == 0))
  {
    fruchtermanReingold(&graph, flags.iter, flags.width, flags.height);
  }
  else if (strcmp(flags.algorithmName, "kamada") == 0)
  {
    kamadaKawaiLayout(&graph, flags.width, flags.height, flags.iter);
  }
  else
  {
    fprintf(stderr, "Błąd! Podana nazwa algorytmu jest nieprawidłowa!\n");
    freeGraph(&graph);
    return -1;
  }

  FILE *outFile = NULL;
  if (flags.isBinary == false)
  {
    outFile = fopen(flags.outputFile, "w");
  }
  else
  {
    outFile = fopen(flags.outputFile, "wb");
  }
  if (outFile == NULL)
  {
    fprintf(stderr, "Błąd! Nie można otworzyć pliku wyjściowego do zapisu!\n");
    freeGraph(&graph); /* zwolnienie pamięci przed wyrzuceniem błędu */
    return -1;
  }

  saveResults(outFile, &graph, flags.isBinary);
  fclose(outFile);
  /* zwolnienie pamięci zaalokowanej na wierzchołki i sąsiadów */
  freeGraph(&graph);
  return 0;
}