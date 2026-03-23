#ifndef PLANARITY_H
#define PLANARITY_H

#include "graph.h"
#include <stdbool.h>

bool isGraphPlanar(Graph *graph);

int makeGraphPlanar(Graph *graph);

#endif