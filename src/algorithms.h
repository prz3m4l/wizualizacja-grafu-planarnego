#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#define MIN_DIST 0.01
#define TEMP_FACTOR 0.95

#include "io_manager.h"
#include <math.h>
#include <stdlib.h>

void fruchterman_reingold(Graph *g, int iterations, double width,
                          double height);
#endif
