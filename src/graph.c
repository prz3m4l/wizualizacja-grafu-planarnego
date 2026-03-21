#include "graph.h"

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
      if (dfsFindCycle(graph, neighbor, current_vertex, parents, cycles,
                       cycle_length, visited)) {
        return true;
      }
    } else if (neighbor != parent_vertex) {
      // Jeśli sąsiad jest odwiedzony i nie jest rodzicem - mamy cykl
      *cycle_length = 0;
      int curr = current_vertex;

      while (curr != neighbor && curr != -1) {
        cycles[*cycle_length] = curr;
        (*cycle_length)++;
        curr = parents[curr];
      }
      cycles[*cycle_length] = neighbor;
      (*cycle_length)++;

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
  bool *in_fragment;
  bool *is_contact;

  int *admissible_faces;
  int admissible_count;
} Fragment;

typedef struct {
  int *boundary_vertices;
  int boundary_count;
} Face;

static int getNeighbourIndex(const Graph *graph, int u, int v) {
  for (int i = 0; i < graph->vertices[u].count; i++) {
    if (graph->vertices[u].neighbours[i] == v) return i;
  }
  return -1;
}

static bool checkFragmentPlanarity(const Graph *graph, Fragment *f) {
  int sub_V = f->vertices_count + f->contact_count;
  if (sub_V < 3)
    return true;

  int *orig_to_sub = malloc(graph->vertices_n * sizeof(int));
  for (int i = 0; i < graph->vertices_n; i++)
    orig_to_sub[i] = -1;

  int current_sub_id = 0;
  for (int i = 0; i < f->vertices_count; i++) {
    orig_to_sub[f->vertices[i]] = current_sub_id++;
  }
  for (int i = 0; i < f->contact_count; i++) {
    orig_to_sub[f->contact_points[i]] = current_sub_id++;
  }

  Graph sub = {0};
  sub.vertices_n = sub_V;
  sub.vertices = calloc(sub_V, sizeof(Node));
  for (int i = 0; i < sub_V; i++)
    sub.vertices[i].v = i;

  for (int i = 0; i < f->vertices_count; i++) {
    int u = f->vertices[i];
    for (int j = 0; j < graph->vertices[u].count; j++) {
      int v = graph->vertices[u].neighbours[j];
      if (f->in_fragment[v]) {
        if (u < v) {
          addVertex(sub.vertices, orig_to_sub[u], orig_to_sub[v]);
          addVertex(sub.vertices, orig_to_sub[v], orig_to_sub[u]);
          sub.edges_n++;
        }
      } else if (f->is_contact[v]) {
        addVertex(sub.vertices, orig_to_sub[u], orig_to_sub[v]);
        addVertex(sub.vertices, orig_to_sub[v], orig_to_sub[u]);
        sub.edges_n++;
      }
    }
  }

  bool result = isGraphPlanar(&sub);

  for (int i = 0; i < sub_V; i++) {
    free(sub.vertices[i].neighbours);
  }
  free(sub.vertices);
  free(orig_to_sub);

  return result;
}

Face *initDmpFaces(int *cycle, int cycle_length, int vertices_num,
                   bool **vertex_drawn_out, int *faces_count_out) {
  bool *vertex_drawn = (bool *)calloc(vertices_num, sizeof(bool));
  if (vertex_drawn == NULL)
    return NULL;

  for (int i = 0; i < cycle_length; i++) {
    vertex_drawn[cycle[i]] = true;
  }
  *vertex_drawn_out = vertex_drawn;

  int max_faces = 2 * vertices_num;
  Face *faces = malloc(max_faces * sizeof(Face));
  if (faces == NULL) {
    free(vertex_drawn);
    return NULL;
  }

  faces[0].boundary_count = cycle_length;
  faces[0].boundary_vertices = malloc(cycle_length * sizeof(int));
  for (int i = 0; i < cycle_length; i++)
    faces[0].boundary_vertices[i] = cycle[i];

  faces[1].boundary_count = cycle_length;
  faces[1].boundary_vertices = malloc(cycle_length * sizeof(int));
  for (int i = 0; i < cycle_length; i++)
    faces[1].boundary_vertices[i] = cycle[i];

  *faces_count_out = 2;
  return faces;
}

static bool isVertexOnFace(int v, const Face *face) {
  for (int i = 0; i < face->boundary_count; i++) {
    if (face->boundary_vertices[i] == v)
      return true;
  }
  return false;
}

void findAdmissibleFaces(Fragment *frag, const Face *faces, int faces_count) {
  frag->admissible_faces = (int *)malloc(faces_count * sizeof(int));
  frag->admissible_count = 0;

  for (int i = 0; i < faces_count; i++) {
    bool can_be_drawn_here = true;
    for (int j = 0; j < frag->contact_count; j++) {
      if (!isVertexOnFace(frag->contact_points[j], &faces[i])) {
        can_be_drawn_here = false;
        break;
      }
    }
    if (can_be_drawn_here) {
      frag->admissible_faces[frag->admissible_count++] = i;
    }
  }
}

static void dfsBuildFragment(const Graph *graph, int u, bool *vertex_drawn,
                             bool **edge_drawn, int **edge_visited, int visit_id,
                             Fragment *frag) {
  for (int i = 0; i < graph->vertices[u].count; i++) {
    int v = graph->vertices[u].neighbours[i];

    // Rozpatrujemy tylko nienarysowane i jeszcze nieodwiedzone w tym kroku
    // krawędzie
    if (!edge_drawn[u][i] && edge_visited[u][i] != visit_id) {
      edge_visited[u][i] = visit_id;
      edge_visited[v][getNeighbourIndex(graph, v, u)] = visit_id;

      if (vertex_drawn[v]) {
        if (!frag->is_contact[v]) {
          frag->is_contact[v] = true;
          frag->contact_points[frag->contact_count++] = v;
        }
      } else {
        if (!frag->in_fragment[v]) {
          frag->in_fragment[v] = true;
          frag->vertices[frag->vertices_count++] = v;
          dfsBuildFragment(graph, v, vertex_drawn, edge_drawn, edge_visited, visit_id,
                           frag);
        }
      }
    }
  }
}

Fragment *getAllFragments(const Graph *graph, bool *vertex_drawn,
                          bool **edge_drawn, int **edge_visited, int visit_id, int *fragments_count) {
  int V = graph->vertices_n;
  int max_frags = graph->edges_n > 0 ? graph->edges_n : 1;
  Fragment *fragments = malloc(max_frags * sizeof(Fragment));
  *fragments_count = 0;

  for (int u = 0; u < V; u++) {
    for (int i = 0; i < graph->vertices[u].count; i++) {
      int v = graph->vertices[u].neighbours[i];

      // Znaleźliśmy krawędź, która nie jest na płaszczyźnie i nie należy
      // jeszcze do żadnego fragmentu
      if (!edge_drawn[u][i] && edge_visited[u][i] != visit_id) {
        Fragment *frag = &fragments[*fragments_count];

        frag->vertices = malloc(V * sizeof(int));
        frag->vertices_count = 0;
        frag->contact_points = malloc(V * sizeof(int));
        frag->contact_count = 0;
        frag->in_fragment = calloc(V, sizeof(bool));
        frag->is_contact = calloc(V, sizeof(bool));
        frag->admissible_faces = NULL;
        frag->admissible_count = 0;

        if (vertex_drawn[u] && vertex_drawn[v]) {
          frag->contact_points[frag->contact_count++] = u;
          frag->is_contact[u] = true;
          frag->contact_points[frag->contact_count++] = v;
          frag->is_contact[v] = true;

          edge_visited[u][i] = visit_id;
          edge_visited[v][getNeighbourIndex(graph, v, u)] = visit_id;
        } else {
          int start_node = vertex_drawn[u] ? v : u;

          if (vertex_drawn[u]) {
            frag->contact_points[frag->contact_count++] = u;
            frag->is_contact[u] = true;
          } else if (vertex_drawn[v]) {
            frag->contact_points[frag->contact_count++] = v;
            frag->is_contact[v] = true;
          }

          if (!vertex_drawn[start_node]) {
            frag->vertices[frag->vertices_count++] = start_node;
            frag->in_fragment[start_node] = true;
          }

          dfsBuildFragment(graph, start_node, vertex_drawn, edge_drawn,
                           edge_visited, visit_id, frag);
        }

        (*fragments_count)++;
      }
    }
  }

  return fragments;
}

static bool dfsFindPath(const Graph *graph, int current, int start_node,
                        Fragment *frag, bool **edge_drawn, bool *visited,
                        int *temp_path, int depth, int **out_path,
                        int *out_length) {
  visited[current] = true;
  temp_path[depth] = current;

  if (frag->is_contact[current] && current != start_node) {
    *out_length = depth + 1;
    *out_path = malloc((*out_length) * sizeof(int));
    for (int i = 0; i < *out_length; i++) {
      (*out_path)[i] = temp_path[i];
    }
    return true;
  }

  for (int i = 0; i < graph->vertices[current].count; i++) {
    int neighbor = graph->vertices[current].neighbours[i];

    // Upewniamy się, że nie przejdziemy po krawędzi już narysowanej na
    // płaszczyźnie
    if (!visited[neighbor] && !edge_drawn[current][i]) {
      if (frag->in_fragment[neighbor] || frag->is_contact[neighbor]) {
        if (dfsFindPath(graph, neighbor, start_node, frag, edge_drawn, visited,
                        temp_path, depth + 1, out_path, out_length)) {
          return true;
        }
      }
    }
  }
  return false;
}

int *findPathBetweenContacts(const Graph *graph, Fragment *frag,
                             bool **edge_drawn, int *path_length) {
  if (frag->vertices_count == 0 && frag->contact_count == 2) {
    *path_length = 2;
    int *path = malloc(2 * sizeof(int));
    path[0] = frag->contact_points[0];
    path[1] = frag->contact_points[1];
    return path;
  }

  if (frag->contact_count < 2) {
    *path_length = 0;
    return NULL;
  }

  bool *visited = calloc(graph->vertices_n, sizeof(bool));
  int *temp_path = malloc(graph->vertices_n * sizeof(int));
  int *final_path = NULL;
  *path_length = 0;

  int start_node = frag->contact_points[0];

  dfsFindPath(graph, start_node, start_node, frag, edge_drawn, visited,
              temp_path, 0, &final_path, path_length);

  free(visited);
  free(temp_path);
  return final_path;
}

void embedPathAndSplitFace(Face **faces_ptr, int *faces_count,
                           int target_face_idx, int *path, int path_length,
                           bool *vertex_drawn, bool **edge_drawn, const Graph *graph) {
  Face *faces = *faces_ptr;
  Face target = faces[target_face_idx];

  int idx_start = -1, idx_end = -1;
  for (int i = 0; i < target.boundary_count; i++) {
    if (target.boundary_vertices[i] == path[0])
      idx_start = i;
    if (target.boundary_vertices[i] == path[path_length - 1])
      idx_end = i;
  }

  int arc1_len =
      (idx_end - idx_start + target.boundary_count) % target.boundary_count + 1;
  int arc2_len =
      (idx_start - idx_end + target.boundary_count) % target.boundary_count + 1;

  int new_face1_count = arc1_len + path_length - 2;
  int *new_boundary1 = malloc(new_face1_count * sizeof(int));
  int c1 = 0;
  for (int i = 0; i < arc1_len; i++) {
    new_boundary1[c1++] =
        target.boundary_vertices[(idx_start + i) % target.boundary_count];
  }
  for (int i = path_length - 2; i >= 1; i--) {
    new_boundary1[c1++] = path[i];
  }

  int new_face2_count = arc2_len + path_length - 2;
  int *new_boundary2 = malloc(new_face2_count * sizeof(int));
  int c2 = 0;
  for (int i = 0; i < arc2_len; i++) {
    new_boundary2[c2++] =
        target.boundary_vertices[(idx_end + i) % target.boundary_count];
  }
  for (int i = 1; i < path_length - 1; i++) {
    new_boundary2[c2++] = path[i];
  }

  free(target.boundary_vertices);
  faces[target_face_idx].boundary_vertices = new_boundary1;
  faces[target_face_idx].boundary_count = new_face1_count;

  faces[*faces_count].boundary_vertices = new_boundary2;
  faces[*faces_count].boundary_count = new_face2_count;
  (*faces_count)++;

  // Oznaczamy i wierzchołki i krawedzie jako narysowane
  for (int i = 1; i < path_length - 1; i++) {
    vertex_drawn[path[i]] = true;
  }
  for (int i = 0; i < path_length - 1; i++) {
    int u = path[i];
    int v = path[i + 1];
    int idx_u = getNeighbourIndex(graph, u, v);
    if (idx_u != -1) edge_drawn[u][idx_u] = true;
    int idx_v = getNeighbourIndex(graph, v, u);
    if (idx_v != -1) edge_drawn[v][idx_v] = true;
  }
}

void freeFragments(Fragment *fragments, int count) {
  if (fragments == NULL)
    return;
  for (int i = 0; i < count; i++) {
    free(fragments[i].vertices);
    free(fragments[i].contact_points);
    free(fragments[i].in_fragment);
    free(fragments[i].is_contact);
    free(fragments[i].admissible_faces);
  }
  free(fragments);
}

bool isGraphPlanar(Graph *graph) {
  int V = graph->vertices_n;
  if (V >= 3) {
    if (graph->edges_n > 3 * V - 6) {
      return false;
    }
  } else {
    return true;
  }

  int *parents = malloc(V * sizeof(int));
  int *cycles = malloc(V * sizeof(int));
  bool *visited = calloc(V, sizeof(bool));
  int cycle_length = 0;

  bool has_cycle = false;
  for (int i = 0; i < V; i++) {
    if (!visited[i]) {
      if (dfsFindCycle(graph, i, -1, parents, cycles, &cycle_length, visited)) {
        has_cycle = true;
        break;
      }
    }
  }

  if (!has_cycle) {
    free(parents);
    free(cycles);
    free(visited);
    return true;
  }

  bool *vertex_drawn = NULL;
  int faces_count = 0;
  Face *faces =
      initDmpFaces(cycles, cycle_length, V, &vertex_drawn, &faces_count);

  bool **edge_drawn = malloc(V * sizeof(bool *));
  int **edge_visited = malloc(V * sizeof(int *));
  for (int i = 0; i < V; i++) {
    edge_drawn[i] = calloc(graph->vertices[i].count, sizeof(bool));
    edge_visited[i] = calloc(graph->vertices[i].count, sizeof(int));
  }
  int visit_id = 0;

  // Oznaczamy krawędzie z głównego cyklu jako narysowane
  for (int i = 0; i < cycle_length; i++) {
    int u = cycles[i];
    int v = cycles[(i + 1) % cycle_length];
    int idx_u = getNeighbourIndex(graph, u, v);
    if (idx_u != -1) edge_drawn[u][idx_u] = true;
    int idx_v = getNeighbourIndex(graph, v, u);
    if (idx_v != -1) edge_drawn[v][idx_v] = true;
  }

  bool is_planar = true;

  while (true) {
    visit_id++;
    int fragments_count = 0;
    Fragment *fragments =
        getAllFragments(graph, vertex_drawn, edge_drawn, edge_visited, visit_id, &fragments_count);

    if (fragments_count == 0) {
      break;
    }

    int chosen_fragment_idx = -1;
    int target_face_idx = -1;

    for (int i = 0; i < fragments_count; i++) {
      findAdmissibleFaces(&fragments[i], faces, faces_count);

      if (fragments[i].admissible_count == 0) {
        is_planar = false;
        break;
      }
      if (fragments[i].admissible_count == 1) {
        chosen_fragment_idx = i;
        target_face_idx = fragments[i].admissible_faces[0];
      }
    }

    if (!is_planar) {
      freeFragments(fragments, fragments_count);
      break;
    }

    if (chosen_fragment_idx == -1) {
      chosen_fragment_idx = 0;
      target_face_idx = fragments[0].admissible_faces[0];
    }

    int path_length = 0;
    int *path = findPathBetweenContacts(graph, &fragments[chosen_fragment_idx],
                                        edge_drawn, &path_length);
    if (path == NULL) {
      Fragment *f = &fragments[chosen_fragment_idx];

      if (!checkFragmentPlanarity(graph, f)) {
        is_planar = false;
        freeFragments(fragments, fragments_count);
        break;
      }

      // Przechodzimy przez wszystkie wierzchołki ogona
      for (int k = 0; k < f->vertices_count; k++) {
        int u = f->vertices[k];

        // Oznaczamy wierzchołek jako narysowany
        vertex_drawn[u] = true;

        // Oznaczamy wszystkie krawędzie wewnątrz tego ogona jako narysowane
        for (int j = 0; j < graph->vertices[u].count; j++) {
          int v = graph->vertices[u].neighbours[j];
          if (f->in_fragment[v] || f->is_contact[v]) {
            edge_drawn[u][j] = true;
            int idx_v = getNeighbourIndex(graph, v, u);
            if (idx_v != -1) edge_drawn[v][idx_v] = true;
          }
        }
      }

      // Zwalniamy pamięć z tej iteracji i przeskakujemy do kolejnej
      freeFragments(fragments, fragments_count);
      continue;
    }

    embedPathAndSplitFace(&faces, &faces_count, target_face_idx, path,
                          path_length, vertex_drawn, edge_drawn, graph);

    free(path);
    freeFragments(fragments, fragments_count);
  }

  free(parents);
  free(cycles);
  free(visited);
  free(vertex_drawn);
  for (int i = 0; i < V; i++) {
    free(edge_drawn[i]);
    free(edge_visited[i]);
  }
  free(edge_drawn);
  free(edge_visited);
  for (int i = 0; i < faces_count; i++)
    free(faces[i].boundary_vertices);
  free(faces);

  return is_planar;
}

static int addExtraEdge(Graph *graph, int v, int u) {
  if (addVertex(graph->vertices, v, u) == -1)
    return -1;
  if (addVertex(graph->vertices, u, v) == -1)
    return -1;

  Edge *tmp = realloc(graph->edges, (graph->edges_n + 1) * sizeof(Edge));
  if (tmp == NULL)
    return -1;
  graph->edges = tmp;

  graph->edges[graph->edges_n].idA = v;
  graph->edges[graph->edges_n].idB = u;
  graph->edges[graph->edges_n].weight = 1.0;
  graph->edges[graph->edges_n].name = strdup("Auto-Generated-Edge");

  graph->edges_n++;
  return 0;
}

int ensureConnectivity(Graph *graph) {
  bool repaired = false;
  if (graph->vertices_n == 0)
    return 1;

  bool *visited = calloc(graph->vertices_n, sizeof(bool));
  if (visited == NULL)
    return -1;

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
  return repaired ? 0 : 1;
}

int makeGraphPlanar(Graph *graph) {
  if (isGraphPlanar(graph)) {
    return 0;
  }
 
  fprintf(stderr, "Ostrzeżenie: Graf nieplanarny! Rozpoczynam naprawę (usuwanie krawędzi)...\n");
 
  int origEdgesCount = graph->edges_n;
  Edge *origEdges = graph->edges;
 
  graph->edges = malloc(origEdgesCount * sizeof(Edge));
  if (graph->edges == NULL) {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas naprawy planarności!\n");
    return -1;
  }
  graph->edges_n = 0;
 
  for (int i = 0; i < graph->vertices_n; i++) {
    graph->vertices[i].count = 0;
  }
 
  /* Tarcza Eulera: graf planarny ma co najwyżej 3v-6 krawędzi */
  int max_edges_limit = (graph->vertices_n >= 3) ? (3 * graph->vertices_n - 6) : origEdgesCount;
 
  int removedEdges = 0;
 
  for (int i = 0; i < origEdgesCount; i++) {
    Edge currentEdge = origEdges[i];
 
    /* Tarcza Eulera: odrzuć bez wywołania isGraphPlanar */
    if (graph->edges_n == max_edges_limit) {
      fprintf(stderr, "Ostrzeżenie: Odrzucono krawędź \"%s\" (%d - %d) aby zapewnić planarność.\n",
              currentEdge.name, currentEdge.idA, currentEdge.idB);
      free(currentEdge.name);
      removedEdges++;
      continue;
    }
 
    graph->edges[graph->edges_n] = currentEdge;
    graph->edges_n++;
    addVertex(graph->vertices, currentEdge.idA, currentEdge.idB);
    addVertex(graph->vertices, currentEdge.idB, currentEdge.idA);
 
    if (!isGraphPlanar(graph)) {
      graph->edges_n--;
 
      Node *na = &graph->vertices[currentEdge.idA];
      for (int j = 0; j < na->count; j++) {
        if (na->neighbours[j] == currentEdge.idB) {
          na->neighbours[j] = na->neighbours[na->count - 1];
          na->count--;
          break;
        }
      }
 
      Node *nb = &graph->vertices[currentEdge.idB];
      for (int j = 0; j < nb->count; j++) {
        if (nb->neighbours[j] == currentEdge.idA) {
          nb->neighbours[j] = nb->neighbours[nb->count - 1];
          nb->count--;
          break;
        }
      }
 
      fprintf(stderr, "Ostrzeżenie: Odrzucono krawędź \"%s\" (%d - %d) aby zapewnić planarność.\n",
              currentEdge.name, currentEdge.idA, currentEdge.idB);
      free(currentEdge.name);
      removedEdges++;
    }
  }
 
  free(origEdges);
 
  fprintf(stderr, "Ostrzeżenie: Naprawa zakończona. Usunięto %d krawędzi.\n", removedEdges);
  return removedEdges;
}