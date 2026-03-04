#include "algorithms.h"

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
    v[i].x = (double)rand() / RAND_MAX * width;
    v[i].y = (double)rand() / RAND_MAX * height;
    v[i].dx = 0;
    v[i].dy = 0;
  }

  for (int iter = 0; iter < iterations; iter++) {
    // Zerowanie sił
    for (int i = 0; i < vn; i++) {
      v[i].dx = 0;
      v[i].dy = 0;
    }

    // Odpychanie
    for (int i = 0; i < vn; i++) {
      for (int j = i + 1; j < vn; j++) {
        // liczenie dystansu
        double deltax = v[i].x - v[j].x;
        double deltay = v[i].y - v[j].y;
        double d2 = deltax * deltax + deltay * deltay; // d^2 zeby uniknac sqrt
        if (d2 < MIN_DIST) // aby uniknac dzielenia przez 0
          d2 = MIN_DIST;

        /*
         * sila odpychania = (k * k) / d
         * przesunięcie = sila * (wektor kierunkowy / d) czyli:
         * (k*k) * (delta / d) = (k*k*delta) / d^2
         */
        double force_factor = (k * k) / d2;
        v[i].dx += deltax * force_factor;
        v[i].dy += deltay * force_factor;
        // Odpychanie w przeciwna stronę
        v[j].dx -= deltax * force_factor;
        v[j].dy -= deltay * force_factor;
      }
    }

    // Przyciąganie
    for (int i = 0; i < en; i++) {
      Node *vA = &v[e[i].idA];
      Node *vB = &v[e[i].idB];
      double deltax = vA->x - vB->x;
      double deltay = vA->y - vB->y;
      double d =
          sqrt(deltax * deltax + deltay * deltay); // d^2 zeby uniknac sqrt
      if (d < MIN_DIST)
        d = MIN_DIST;

      // f_a = d^2 / k -> składowa = (d^2 / k) * (delta / d) = (d * delta) / k
      double force_factor = (d / k) * e[i].weight;
      vA->dx -= deltax * force_factor;
      vA->dy -= deltay * force_factor;
      // Przyciąganie w przeciwna stronę
      vB->dx += deltax * force_factor;
      vB->dy += deltay * force_factor;
    }

    // Ruch
    for (int i = 0; i < vn; i++) {
      double dist = v[i].dx * v[i].dx + v[i].dy * v[i].dy;
      v[i].x += (v[i].dx / dist) * fmin(dist, t);
      v[i].y += (v[i].dy / dist) * fmin(dist, t);

      // Bariery(jakby cos chcialo wyjsc)
      if (v[i].x < 0)
        v[i].x = 0;
      if (v[i].x > width)
        v[i].x = width;
      if (v[i].y < 0)
        v[i].y = 0;
      if (v[i].y > height)
        v[i].y = height;
    }

    // Limitowanie Temperatura
    t *= TEMP_FACTOR;
  }
}
