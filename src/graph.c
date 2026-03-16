#include "graph.h"
#include <stdbool.h>

int addVertex(Node *vertices, int v, int u) {
  Node *vertex = &vertices[v];
  vertex->v = v;
  if (vertex->size == 0) {
    vertex->size = 2;
    vertex->neighbours = malloc(vertex->size * sizeof(int));
    vertex->count = 0;
  } else if (vertex->size <= vertex->count) {
    vertex->size *= 2;
    int *tmp = realloc(vertex->neighbours, vertex->size * sizeof(int));
    if (tmp == NULL) {
      fprintf(stderr,
              "Błąd! Nie można zaalokować pamięci dla sąsiadów wierzchołka!\n");
      return -1;
    } else {
      vertex->neighbours = tmp;
    }
  }
  vertex->neighbours[vertex->count] = u;
  vertex->count++;
  return 0;
}

static void dfsVisit(const Graph *graph, int current_vertex, bool *visited) {
  visited[current_vertex] = true;
  for (int i = 0; i < (graph->vertices[current_vertex].count); i++) {
    int neighbor = graph->vertices[current_vertex].neighbours[i];
    if (visited[neighbor] == false) {
      dfsVisit(graph, neighbor, visited);
    }
  }
}

static bool dfsFindCycle(const Graph *graph, int current_vertex,
                         int parent_vertex, int *parents, int *cycles,
                         int *cycle_length, bool *visited) {
  visited[current_vertex] = true;
  parents[current_vertex] = parent_vertex;

  for (int i = 0; i < (graph->vertices[current_vertex].count); i++) {
    int neighbor = graph->vertices[current_vertex].neighbours[i];
    if (visited[neighbor] == false) {
      if (dfsFindCycle(graph, neighbor, parent_vertex, parents, cycles,
                       cycle_length, visited)) {
        return true;
      }

    } else if (neighbor != parent_vertex) { // Jeśli sąsiad jest odwiedzony i
                                            // nie jest rodzicem mamy cykl

      *cycle_length = 0;
      int curr = current_vertex;

      while (curr != neighbor && curr != 1) {
        cycles[*cycle_length++] = curr;
        curr = parents[curr];
      }
      cycles[*cycle_length++] = neighbor;

      return true;
    }
  }
  return false;
}

typedef struct {
  int *vertices;
  int vertices_count;

  int *contact_points;
  int contact_count;

  int *admissible_faces;
  int admissible_count;
} Fragment;

static void dfsFindFragment(const Graph *graph, int current_vertex,
                            bool *is_drawn, bool *visited_frag,
                            Fragment *frag) {
  visited_frag[current_vertex] = true;
  frag->vertices[frag->vertices_count++] = current_vertex;

  for (int i = 0; i < graph->vertices[current_vertex].count; i++) {
    int neighbor = graph->vertices[current_vertex].neighbours[i];

    if (is_drawn[neighbor]) { // jesli narysowany zapisujemy jako contanct point
                              // unikajac duplikatów
      bool already_added = false;
      for (int j = 0; j < frag->contact_count; j++) {
        if (frag->contact_points[j] == neighbor) {
          already_added = true;
          break;
        }
      }
      if (!already_added) {
        frag->contact_points[frag->contact_count++] = neighbor;
      }
    } else if (!visited_frag[neighbor]) {
      dfsFindFragment(graph, neighbor, is_drawn, visited_frag, frag);
    }
  }
}

typedef struct {
  int *boundary_vertices;
  int boundary_count;
} Face;

Face *initDmpFaces(int *cycle, int cycle_length, int vertices_num,
                   bool **is_drawn_out, int *faces_count_out) {
  bool *is_drawn = (bool *)calloc(vertices_num, sizeof(bool));
  if (is_drawn == NULL)
    return NULL;

  // Oznaczamy wierzchołki z cyklu
  for (int i = 0; i < cycle_length; i++) {
    is_drawn[cycle[i]] = true;
  }

  *is_drawn_out = is_drawn;

  // Maksymalna liczba ścian to 2 * num_vertices, z Eulera
  int max_faces = 2 * vertices_num;
  Face *faces = malloc(max_faces * sizeof(Face));
  if (faces == NULL) {
    free(is_drawn);
    return NULL;
  }

  // Sciana wewnętrzna
  faces[0].boundary_count = cycle_length;
  faces[0].boundary_vertices = malloc(cycle_length * sizeof(int));
  for (int i = 0; i < cycle_length; i++) {
    faces[0].boundary_vertices[i] = cycle[i];
  }

  // Sciana zewnętrzna
  faces[1].boundary_count = cycle_length;
  faces[1].boundary_vertices = malloc(cycle_length * sizeof(int));
  for (int i = 0; i < cycle_length; i++) {
    faces[1].boundary_vertices[i] = cycle[i];
  }

  *faces_count_out = 2; // Zapisujemy, że mamy na start 2 ściany

  return faces;
}

static bool isVertexOnFace(int v, const Face *face) {
  for (int i = 0; i < face->boundary_count; i++) {
    if (face->boundary_vertices[i] == v) {
      return true;
    }
  }
  return false;
}

void findAdmissibleFaces(Fragment *frag, const Face *faces, int faces_count) {
  frag->admissible_faces = (int *)malloc(faces_count * sizeof(int));
  frag->admissible_count = 0;

  for (int i = 0; i < faces_count; i++) {
    bool can_be_drawn_here = true;
    for (int j = 0; j < frag->contact_count; j++) {

      // Jeśli chociaż 1 punkt styku nie leży na obwodzie tej ściany to
      // odrzucamy
      if (!isVertexOnFace(frag->contact_points[j], &faces[i])) {
        can_be_drawn_here = false;
        break;
      }
    }

    // Jeśli wszystkie punkty styku były na obwodzie ściana jest dopuszczalna
    if (can_be_drawn_here) {
      frag->admissible_faces[frag->admissible_count++] = i;
    }
  }
}

static int addExtraEdge(Graph *graph, int v, int u) {
  /* Aktualizacja listy sąsiedztwa */
  if (addVertex(graph->vertices, v, u) == -1) {
    return -1;
  }
  if (addVertex(graph->vertices, u, v) == -1) {
    return -1;
  }

  /* Zwiększenie tablicy krawędzi dla algorytmu rysującego */
  Edge *tmp = realloc(graph->edges, (graph->edges_n + 1) * sizeof(Edge));
  if (tmp == NULL) {
    return -1;
  }
  graph->edges = tmp;

  /* Uzupełnienie danych nowej krawędzi */
  graph->edges[graph->edges_n].idA = v;
  graph->edges[graph->edges_n].idB = u;
  graph->edges[graph->edges_n].weight = 1.0;
  graph->edges[graph->edges_n].name = strdup("Auto-Generated-Edge");

  graph->edges_n++;
  return 0;
}

int ensureConnectivity(Graph *graph) {
  bool repaired = false;
  if (graph->vertices_n == 0) {
    return 1;
  }
  bool *visited = calloc(graph->vertices_n, sizeof(bool));
  if (visited == NULL) {
    return -1;
  }
  dfsVisit(graph, 0, visited);
  for (int i = 0; i < graph->vertices_n; i++) {
    if (visited[i] == false) {
      repaired = true;
      if (addExtraEdge(graph, 0, i) == -1) {
        free(visited);
        return -1;
      }
      dfsVisit(graph, i, visited);
    }
  }
  free(visited);
  if (repaired) {
    return 0;
  }
  return 1;
}
