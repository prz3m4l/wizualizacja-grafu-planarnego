#include "algorithms.h"

void fruchterman_reingold(Graph *graph, int iterations, double width,
                          double height) {
  int vn = graph->vertices_n;
  double k = sqrt((width * height) / vn);
  double t = width / 10; // Temperatura poczatkowa

  for (int iter = 0; iter < iterations; iter++) {
    // Zerowanie sił
    for (int i = 0; i < vn; i++) {
      graph->dx[i] = 0.0;
      graph->dy[i] = 0.0;
    }

    // Odpychanie
    for (int i = 0; i < vn; i++) {
      for (int j = i + 1; j < vn; j++) {
        // liczenie dystansu
        double deltax = graph->x[i] - graph->x[j];
        double deltay = graph->y[i] - graph->y[j];
        double d2 = deltax * deltax + deltay * deltay; // d^2 zeby uniknac sqrt
        if (d2 < MIN_DIST) // aby uniknac dzielenia przez 0
          d2 = MIN_DIST;

        /*
         * sila odpychania = (k * k) / d
         * przesunięcie = sila * (wektor kierunkowy / d) czyli:
         * (k*k) * (delta / d) = (k*k*delta) / d^2
         */
        double force_factor = (k * k) / d2;
        graph->dx[i] += deltax * force_factor;
        graph->dy[i] += deltay * force_factor;
        // Odpychanie w przeciwna stronę
        graph->dx[j] -= deltax * force_factor;
        graph->dy[j] -= deltay * force_factor;
      }
    }

    // Przyciąganie
    for (int i = 0; i < graph->edges_n; i++) {
      int idA = graph->edges[i].idA;
      int idB = graph->edges[i].idB;

      double deltax = graph->x[idA] - graph->x[idB];
      double deltay = graph->y[idA] - graph->y[idB];
      double d = sqrt(deltax * deltax + deltay * deltay);
      if (d < MIN_DIST)
        d = MIN_DIST;

      // f_a = d^2 / k -> składowa = (d^2 / k) * (delta / d) = (d * delta) / k
      double force_factor = (d / k);
      graph->dx[idA] -= deltax * force_factor;
      graph->dy[idA] -= deltay * force_factor;
      // Przyciąganie w przeciwna stronę
      graph->dx[idB] += deltax * force_factor;
      graph->dy[idB] += deltay * force_factor;
    }

    // Ruch
    for (int i = 0; i < vn; i++) {
      double dist =
          sqrt(graph->dx[i] * graph->dx[i] + graph->dy[i] * graph->dy[i]);
      if (dist > 0) {
        graph->x[i] += (graph->dx[i] / dist) * fmin(dist, t);
        graph->y[i] += (graph->dy[i] / dist) * fmin(dist, t);
      }

      // Bariery(jakby cos chcialo wyjsc)
      if (graph->x[i] < 0)
        graph->x[i] = 0;
      if (graph->x[i] > width)
        graph->x[i] = width;
      if (graph->y[i] < 0)
        graph->y[i] = 0;
      if (graph->y[i] > height)
        graph->y[i] = height;
    }

    // Limitowanie Temperatura
    t *= TEMP_FACTOR;
  }
}

int **calculate_distances(Graph *graph){
  int **matrixDistances = malloc(graph->vertices_n * sizeof(int*));
  if(matrixDistances == NULL){
    return NULL;
  }
  for(int i = 0; i < graph->vertices_n; i++){
    matrixDistances[i] = malloc(graph->vertices_n * sizeof(int));
    if(matrixDistances[i] == NULL){
      for(int k = i - 1; k >= 0; k--){
        free(matrixDistances[k]);
      }
      free(matrixDistances);
      return NULL;
    }
    for(int j = 0; j < graph->vertices_n; j++){
      matrixDistances[i][j] = -1;
    }
  }

  int *queue = malloc(graph->vertices_n * sizeof(int));
  if(queue == NULL){
    for(int i = 0; i < graph->vertices_n; i++){
      free(matrixDistances[i]);
    }
    free(matrixDistances);
    return NULL;
  }

  for(int i = 0; i < graph->vertices_n; i++){
    matrixDistances[i][i] = 0;
    int head = 0;
    int tail = 0;
    queue[tail] = i;
    tail++;
    while(head < tail){
      int current = queue[head];
      head++;
      for(int j = 0; j < graph->vertices[current].count; j++){
        int neighbour = graph->vertices[current].neighbours[j];
        if(matrixDistances[i][neighbour] == -1){
          matrixDistances[i][neighbour] = matrixDistances[i][current] + 1;
          queue[tail] = neighbour;
          tail++;
        }
      }
    }
  }

  free(queue);
  return matrixDistances;
}

void kamada_kawai_layout(Graph *graph, int width, int height, int iterations){
  int **distances = calculate_distances(graph);
  if(distances == NULL){
    fprintf(stderr, "Błąd! Brak pamięci na alokację tablicy odległości!\n");
    return;
  }
  int maxDistance = 0;
  for(int i = 0; i<graph->vertices_n; i++){
    for(int j = 0; j<graph->vertices_n; j++){
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
  Spring **springs = malloc(graph->vertices_n * sizeof(Spring*));
  if(springs == NULL){
    fprintf(stderr, "Błąd! Brak pamięci na macierz sprężyn!\n");
    for(int i = 0; i<graph->vertices_n; i++){
      free(distances[i]);
    }
    free(distances);
    return;
  }
  for(int i = 0; i < graph->vertices_n; i++){
    springs[i] = malloc(graph->vertices_n * sizeof(Spring));
    if(springs[i] == NULL){
      fprintf(stderr, "Błąd! Brak pamięci na wiersz macierzy sprężyn!\n");
      for(int j = i - 1; j >= 0; j--){
        free(springs[i]);
      }
      free(springs);
      for(int j = 0; j<graph->vertices_n; j++){
        free(distances[j]);
      }
      free(distances);
      return;
    }
  }
  for(int i = 0; i < graph->vertices_n; i++){
    for(int j = 0; j < graph->vertices_n; j++){
      if(i == j){
        springs[i][j].l = springs[i][j].k = 0.0;
        continue;
      }
      springs[i][j].l = idealLengthOfSingleEdge * distances[i][j];
      springs[i][j].k = K/(distances[i][j]*distances[i][j]);
    }
  }
  for(int i = 0; i<graph->vertices_n; i++){
    free(distances[i]);
  }
  free(distances);

  for(int i = 0; i<graph->vertices_n; i++){
    graph->x[i] = ((double)rand() / RAND_MAX) * width;
    graph->y[i] = ((double)rand() / RAND_MAX) * height;
  }

  double learning_rate = 0.1; // Temperatura
  double deltaX = 0.0; 
  double deltaY = 0.0;
  double dist = 0.0; // Odległość na ekranie
  double displacement = 0.0; // Jak długa teraz jest
  double F = 0.0; // Siła sprężyny
  for(int iter = 0; iter < iterations; iter++){
    /* Wyzerowanie siły z poprzedniej iteracji */
    for(int i = 0; i < graph->vertices_n; i++){
      graph->dx[i] = 0.0;
      graph->dy[i] = 0.0;
    }
    /* Liczenie siły dla każdej pary wierzchołków */
    for(int i = 0; i < graph->vertices_n; i++){
      for(int j = 0; j < graph->vertices_n; j++){
        if(i == j){
          continue;
        }
        deltaX = graph->x[j] - graph->x[i];
        deltaY = graph->y[j] - graph->y[i];
        dist = sqrt((deltaX*deltaX) + (deltaY*deltaY));
        if(dist < 0.0001) {
            dist = 0.0001;
        }
        displacement = dist - springs[i][j].l;
        F = springs[i][j].k * displacement;
        graph->dx[i] = graph->dx[i] + F * (deltaX/dist);
        graph->dy[i] = graph->dy[i] + F * (deltaY/dist);
      }
    }
    /* Przesuwanie wierzchołków o wyliczone siły */
    for(int i = 0; i < graph->vertices_n; i++){
      graph->x[i] = graph->x[i] + (graph->dx[i] * learning_rate);
      graph->y[i] = graph->y[i] + (graph->dy[i] * learning_rate); 

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
    learning_rate = learning_rate * 0.99; /* Aby wierzchołki poruszały się coraz lżej */
  }
  for(int i = 0; i < graph->vertices_n; i++){
    free(springs[i]);
  }
  free(springs);
}
