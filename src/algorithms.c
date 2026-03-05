#include "algorithms.h"
#include <math.h>

void fruchterman_reingold(Graph *g, int iterations, double width,
                          double height) {
  double k = sqrt((width * height) / g->vertices_n);
  double t = width / 10; // Temperatura poczatkowa

  Node *v = g->vertices;
  Edge *e = g->edges;
  int vn = g->vertices_n;
  int en = g->edges_n;
  // Losowanie pozycji wierzcholkow
  for (int i = 0; i < vn; i++) {
    g->x[i] = (double)rand() / RAND_MAX * width;
    g->y[i] = (double)rand() / RAND_MAX * height;
    g->dx[i] = 0;
    g->dy[i] = 0;
  }

  for (int iter = 0; iter < iterations; iter++) {
    // Zerowanie sił
    for (int i = 0; i < vn; i++) {
      g->dx[i] = 0;
      g->dy[i] = 0;
    }

    // Odpychanie
    for (int i = 0; i < vn; i++) {
      for (int j = i + 1; j < vn; j++) {
        // liczenie dystansu
        double deltax = g->x[i] - g->x[j];
        double deltay = g->y[i] - g->y[j];
        double d2 = deltax * deltax + deltay * deltay; // d^2 zeby uniknac sqrt
        if (d2 < MIN_DIST) // aby uniknac dzielenia przez 0
          d2 = MIN_DIST;

        /*
         * sila odpychania = (k * k) / d
         * przesunięcie = sila * (wektor kierunkowy / d) czyli:
         * (k*k) * (delta / d) = (k*k*delta) / d^2
         */
        double force_factor = (k * k) / d2;
        g->dx[i] += deltax * force_factor;
        g->dy[i] += deltay * force_factor;
        // Odpychanie w przeciwna stronę
        g->dx[j] -= deltax * force_factor;
        g->dy[j] -= deltay * force_factor;
      }
    }

    // Przyciąganie
    for (int i = 0; i < en; i++) {
      int idA = g->edges[i].idA;
      int idB = g->edges[i].idB;

      double deltax = g->x[idA] - g->x[idB];
      double deltay = g->y[idA] - g->y[idB];
      double d = sqrt(deltax * deltax + deltay * deltay);
      if (d < MIN_DIST)
        d = MIN_DIST;

      // f_a = d^2 / k -> składowa = (d^2 / k) * (delta / d) = (d * delta) / k
      double force_factor = (d / k) * e[i].weight;
      g->dx[idA] -= deltax * force_factor;
      g->dy[idA] -= deltay * force_factor;
      // Przyciąganie w przeciwna stronę
      g->dx[idB] += deltax * force_factor;
      g->dy[idB] += deltay * force_factor;
    }

    // Ruch
    for (int i = 0; i < vn; i++) {
      double dist = sqrt(g->dx[i] * g->dx[i] + g->dy[i] * g->dy[i]);
      if (dist > 0) {
        g->x[i] += (g->dx[i] / dist) * fmin(dist, t);
        g->y[i] += (g->dy[i] / dist) * fmin(dist, t);
      }

      // Bariery(jakby cos chcialo wyjsc)
      if (g->x[i] < 0)
        g->x[i] = 0;
      if (g->x[i] > width)
        g->x[i] = width;
      if (g->y[i] < 0)
        g->y[i] = 0;
      if (g->y[i] > height)
        g->y[i] = height;
    }

    // Limitowanie Temperatura
    t *= TEMP_FACTOR;
  }
}
