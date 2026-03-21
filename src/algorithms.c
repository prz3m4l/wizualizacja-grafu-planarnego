#include "algorithms.h"

void fruchtermanReingold(Graph *graph, int iterations, double width,
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
         * (k * k) * (delta / d) = (k * k * delta) / d^2
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
      double screen_distance =
          sqrt(graph->dx[i] * graph->dx[i] + graph->dy[i] * graph->dy[i]);
      if (screen_distance > 0) {
        graph->x[i] += (graph->dx[i] / screen_distance) * fmin(screen_distance, t);
        graph->y[i] += (graph->dy[i] / screen_distance) * fmin(screen_distance, t);
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

int **calculateDistances(Graph *graph){
  int **distanceMatrix = malloc(graph->vertices_n * sizeof(int*));
  if(distanceMatrix == NULL){
    return NULL;
  }
  for(int i = 0; i < graph->vertices_n; i++){
    distanceMatrix[i] = malloc(graph->vertices_n * sizeof(int));
    if(distanceMatrix[i] == NULL){
      for (int j = i - 1; j >= 0; j--) {
        free(distanceMatrix[j]);
      }
      free(distanceMatrix);
      return NULL;
    }
    for(int j = 0; j < graph->vertices_n; j++){
      distanceMatrix[i][j] = -1;
    }
  }

  int *queue = malloc(graph->vertices_n * sizeof(int));
  if(queue == NULL){
    for(int i = 0; i < graph->vertices_n; i++){
      free(distanceMatrix[i]);
    }
    free(distanceMatrix);
    return NULL;
  }

  for(int i = 0; i < graph->vertices_n; i++){
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
        free(springs[j]);
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
  for(int i = 0; i<graph->vertices_n; i++){
    free(distances[i]);
  }
  free(distances);

  for(int i = 0; i<graph->vertices_n; i++){
    graph->x[i] = ((double)rand() / RAND_MAX) * width;
    graph->y[i] = ((double)rand() / RAND_MAX) * height;
  }

  double learning_rate = 0.1; // Temperatura
  for(int iter = 0; iter < iterations; iter++){
    double deltaX = 0.0; 
    double deltaY = 0.0;
    double screen_distance = 0.0; // Odległość na ekranie
    double screen_stretch = 0.0; // Jak długa teraz jest
    double spring_force = 0.0; // Siła sprężyny
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
        screen_distance = sqrt((deltaX*deltaX) + (deltaY*deltaY));
        if(screen_distance < 0.0001) {
            screen_distance = 0.0001;
        }
        screen_stretch = screen_distance - springs[i][j].length;
        spring_force = springs[i][j].stiffness * screen_stretch;
        graph->dx[i] = graph->dx[i] + spring_force * (deltaX/screen_distance);
        graph->dy[i] = graph->dy[i] + spring_force * (deltaY/screen_distance);
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
