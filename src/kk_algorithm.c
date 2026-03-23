#include "kk_algorithm.h"
#include <math.h>
#include <stdlib.h>

/* Parametry sprężyny między parą wierzchołków (algorytm Kamady-Kawai) */
typedef struct {
    double length;    /* idealna długość sprężyny (l_ij) */
    double stiffness; /* stała sprężystości (k_ij) */
} Spring;

static int **calculateDistances(Graph *graph){
  int **distanceMatrix = malloc(graph->verticesCount * sizeof(int*));
  if(distanceMatrix == NULL){
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia macierzy odległości!\n");
    return NULL;
  }
  for(int i = 0; i < graph->verticesCount; i++){
    distanceMatrix[i] = malloc(graph->verticesCount * sizeof(int));
    if(distanceMatrix[i] == NULL){
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia wiersza macierzy odległości!\n");
      for (int j = i - 1; j >= 0; j--) {
        free(distanceMatrix[j]);
      }
      free(distanceMatrix);
      return NULL;
    }
    for(int j = 0; j < graph->verticesCount; j++){
      distanceMatrix[i][j] = -1;
    }
  }

  int *queue = malloc(graph->verticesCount * sizeof(int));
  if(queue == NULL){
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas obliczania odległości (kolejka)!\n");
    for(int i = 0; i < graph->verticesCount; i++){
      free(distanceMatrix[i]);
    }
    free(distanceMatrix);
    return NULL;
  }

  for(int i = 0; i < graph->verticesCount; i++){
    distanceMatrix[i][i] = 0;
    int head = 0;
    int tail = 0;
    queue[tail] = i;
    tail++;
    while(head < tail){
      int current = queue[head];
      head++;
      for(int j = 0; j < graph->vertices[current].count; j++){
        int neighbour = graph->vertices[current].neighbours[j];
        if(distanceMatrix[i][neighbour] == -1){
          distanceMatrix[i][neighbour] = distanceMatrix[i][current] + 1;
          queue[tail] = neighbour;
          tail++;
        }
      }
    }
  }

  free(queue);
  return distanceMatrix;
}

void kamadaKawaiLayout(Graph *graph, int width, int height, int iterations){
  int **distances = calculateDistances(graph);
  if(distances == NULL){
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas zbierania wyników macierzy!\n");
    return;
  }
  int maxDistance = 0;
  for(int i = 0; i<graph->verticesCount; i++){
    for(int j = 0; j<graph->verticesCount; j++){
      if(distances[i][j] > maxDistance){
        maxDistance = distances[i][j];
      }
    }
  }
  /* Zabezpieczenie przed dzieleniem przez 0 */
  if(maxDistance == 0){
    maxDistance = 1; 
  }
  double idealLengthOfSingleEdge = (double)(width < height ? width : height) / maxDistance;
  Spring **springs = malloc(graph->verticesCount * sizeof(Spring*));
  if(springs == NULL){
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas generowania macierzy sprężyn!\n");
    for(int i = 0; i<graph->verticesCount; i++){
      free(distances[i]);
    }
    free(distances);
    return;
  }
  for(int i = 0; i < graph->verticesCount; i++){
    springs[i] = malloc(graph->verticesCount * sizeof(Spring));
    if(springs[i] == NULL){
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas generowania wiersza macierzy sprężyn!\n");
      for(int j = i - 1; j >= 0; j--){
        free(springs[j]);
      }
      free(springs);
      for(int j = 0; j<graph->verticesCount; j++){
        free(distances[j]);
      }
      free(distances);
      return;
    }
  }
  for(int i = 0; i < graph->verticesCount; i++){
    for(int j = 0; j < graph->verticesCount; j++){
      if(i == j){
        springs[i][j].length = springs[i][j].stiffness = 0.0;
        continue;
      }

      if (distances[i][j] <= 0) {
        springs[i][j].length = 0.0;
        springs[i][j].stiffness = 0.0;
        continue;
      }

      springs[i][j].length = idealLengthOfSingleEdge * distances[i][j];
      springs[i][j].stiffness = K/(distances[i][j]*distances[i][j]);
    }
  }
  for(int i = 0; i<graph->verticesCount; i++){
    free(distances[i]);
  }
  free(distances);

  for(int i = 0; i<graph->verticesCount; i++){
    graph->x[i] = ((double)rand() / RAND_MAX) * width;
    graph->y[i] = ((double)rand() / RAND_MAX) * height;
  }

  double learningRate = 0.1; // Temperatura
  for(int iter = 0; iter < iterations; iter++){
    double deltaX = 0.0; 
    double deltaY = 0.0;
    double screenDistance = 0.0; // Odległość na ekranie
    double screenStretch = 0.0; // Jak długa teraz jest
    double springForce = 0.0; // Siła sprężyny
    /* Wyzerowanie siły z poprzedniej iteracji */
    for(int i = 0; i < graph->verticesCount; i++){
      graph->dx[i] = 0.0;
      graph->dy[i] = 0.0;
    }
    /* Liczenie siły dla każdej pary wierzchołków */
    for(int i = 0; i < graph->verticesCount; i++){
      for(int j = 0; j < graph->verticesCount; j++){
        if(i == j){
          continue;
        }
        deltaX = graph->x[j] - graph->x[i];
        deltaY = graph->y[j] - graph->y[i];
        screenDistance = sqrt((deltaX*deltaX) + (deltaY*deltaY));
        if(screenDistance < 0.0001) {
            screenDistance = 0.0001;
        }
        screenStretch = screenDistance - springs[i][j].length;
        springForce = springs[i][j].stiffness * screenStretch;
        graph->dx[i] = graph->dx[i] + springForce * (deltaX/screenDistance);
        graph->dy[i] = graph->dy[i] + springForce * (deltaY/screenDistance);
      }
    }
    /* Przesuwanie wierzchołków o wyliczone siły */
    for(int i = 0; i < graph->verticesCount; i++){
      graph->x[i] = graph->x[i] + (graph->dx[i] * learningRate);
      graph->y[i] = graph->y[i] + (graph->dy[i] * learningRate); 

      if (graph->x[i] < 0){
        graph->x[i] = 0;
      }else if (graph->x[i] > width){
        graph->x[i] = width;
      }
      if (graph->y[i] < 0){
        graph->y[i] = 0;
      }else if (graph->y[i] > height){
        graph->y[i] = height;
      }
    }
    learningRate = learningRate * 0.99; /* Aby wierzchołki poruszały się coraz lżej */
  }
  for(int i = 0; i < graph->verticesCount; i++){
    free(springs[i]);
  }
  free(springs);
}
