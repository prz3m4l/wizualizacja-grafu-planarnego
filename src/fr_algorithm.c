#include "fr_algorithm.h"
#include <math.h>

void fruchtermanReingold(Graph *graph, int iterations, double width,
                         double height)
{
  int vn = graph->verticesCount;
  double k = sqrt((width * height) / vn);
  double t = width / 10; // Temperatura poczatkowa

  for (int iter = 0; iter < iterations; iter++)
  {
    // Zerowanie sił
    for (int i = 0; i < vn; i++)
    {
      graph->dx[i] = 0.0;
      graph->dy[i] = 0.0;
    }

    // Odpychanie
    for (int i = 0; i < vn; i++)
    {
      for (int j = i + 1; j < vn; j++)
      {
        // liczenie dystansu
        double deltax = graph->x[i] - graph->x[j];
        double deltay = graph->y[i] - graph->y[j];
        double d2 = deltax * deltax + deltay * deltay; // d^2 zeby uniknac sqrt
        if (d2 < MIN_DIST)                             // aby uniknac dzielenia przez 0
          d2 = MIN_DIST;

        /*
         * sila odpychania = (k * k) / d
         * przesunięcie = sila * (wektor kierunkowy / d) czyli:
         * (k * k) * (delta / d) = (k * k * delta) / d^2
         */
        double forceFactor = (k * k) / d2;
        graph->dx[i] += deltax * forceFactor;
        graph->dy[i] += deltay * forceFactor;
        // Odpychanie w przeciwna stronę
        graph->dx[j] -= deltax * forceFactor;
        graph->dy[j] -= deltay * forceFactor;
      }
    }

    // Przyciąganie
    for (int i = 0; i < graph->edgesCount; i++)
    {
      int idA = graph->edges[i].idA;
      int idB = graph->edges[i].idB;

      double deltax = graph->x[idA] - graph->x[idB];
      double deltay = graph->y[idA] - graph->y[idB];
      double d = sqrt(deltax * deltax + deltay * deltay);
      if (d < MIN_DIST)
        d = MIN_DIST;

      // f_a = d^2 / k -> składowa = (d^2 / k) * (delta / d) = (d * delta) / k
      double forceFactor = (d / k);
      graph->dx[idA] -= deltax * forceFactor;
      graph->dy[idA] -= deltay * forceFactor;
      // Przyciąganie w przeciwna stronę
      graph->dx[idB] += deltax * forceFactor;
      graph->dy[idB] += deltay * forceFactor;
    }

    // Ruch
    for (int i = 0; i < vn; i++)
    {
      double screenDistance =
          sqrt(graph->dx[i] * graph->dx[i] + graph->dy[i] * graph->dy[i]);
      if (screenDistance > 0)
      {
        graph->x[i] += (graph->dx[i] / screenDistance) * fmin(screenDistance, t);
        graph->y[i] += (graph->dy[i] / screenDistance) * fmin(screenDistance, t);
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
