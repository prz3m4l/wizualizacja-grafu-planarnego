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
