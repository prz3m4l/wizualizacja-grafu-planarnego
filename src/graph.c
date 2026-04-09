#include "graph.h"

/* Funkcja dodająca krawędź między wierzchołkami v i u
 * poprzez dodanie u do listy sąsiadów wierzchołka v.
 * Dynamicznie alokuje lub reallokuje pamięć w razie potrzeby. */
int addVertex(Node *vertices, int v, int u)
{
  Node *vertex = &vertices[v];
  if (vertex->size == 0)
  {
    vertex->size = 2;
    vertex->neighbours = malloc(vertex->size * sizeof(int));
    if (vertex->neighbours == NULL)
    {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia relacji wierzchołka!\n");
      return -1;
    }
    vertex->count = 0;
  }
  else if (vertex->size <= vertex->count)
  {
    int newSize = vertex->size * 2;
    int *tmp = realloc(vertex->neighbours, newSize * sizeof(int));
    if (tmp == NULL)
    {
      fprintf(stderr,
              "Błąd! Nie można zaalokować pamięci podczas poszerzania sąsiadów wierzchołka!\n");
      return -1;
    }
    vertex->neighbours = tmp;
    vertex->size = newSize; // Dopiero po sukcesie aktualizujemy rozmiar
  }
  vertex->neighbours[vertex->count++] = u;
  return 0;
}

/* Funkcja zwalniająca całą pamięć zaalokowaną dla struktury grafu,
 * włączając w to listy sąsiedztwa, krawędzie oraz tablice współrzędnych. */
void freeGraph(Graph *graph)
{
  if (!graph)
    return;

  if (graph->vertices)
  {
    for (int i = 0; i < graph->verticesCount; ++i)
    {
      free(graph->vertices[i].name);
      graph->vertices[i].name = NULL;
      free(graph->vertices[i].neighbours);
      graph->vertices[i].neighbours = NULL;
    }
    free(graph->vertices);
  }

  if (graph->edges)
  {
    for (int i = 0; i < graph->edgesCount; i++)
      free(graph->edges[i].name);
    free(graph->edges);
  }
  graph->edges = NULL;
  graph->vertices = NULL;
  graph->verticesCount = graph->edgesCount = 0;
  free(graph->x);
  free(graph->y);
  free(graph->dx);
  free(graph->dy);
  graph->x = NULL;
  graph->y = NULL;
  graph->dx = NULL;
  graph->dy = NULL;
}

/* Pomocnicza funkcja realizująca przeszukiwanie grafu w głąb (DFS).
 * Oznacza odwiedzone wierzchołki w tablicy visited. */
static void dfsVisit(const Graph *graph, int currentVertex, bool *visited)
{
  visited[currentVertex] = true;
  for (int i = 0; i < (graph->vertices[currentVertex].count); i++)
  {
    int neighbor = graph->vertices[currentVertex].neighbours[i];
    if (visited[neighbor] == false)
    {
      dfsVisit(graph, neighbor, visited);
    }
  }
}

/* Funkcja dodająca dodatkową krawędź do grafu w celu zapewnienia spójności.
 * Aktualizuje zarówno listę sąsiedztwa, jak i tablicę krawędzi. */
static int addExtraEdge(Graph *graph, int v, int u)
{
  if (addVertex(graph->vertices, v, u) == -1)
    return -1;
  if (addVertex(graph->vertices, u, v) == -1)
    return -1;

  Edge *tmp = realloc(graph->edges, (graph->edgesCount + 1) * sizeof(Edge));
  if (tmp == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas dodawania dodatkowej krawędzi!\n");
    return -1;
  }
  graph->edges = tmp;

  graph->edges[graph->edgesCount].idA = v;
  graph->edges[graph->edgesCount].idB = u;
  graph->edges[graph->edgesCount].name = strdup("Auto-Generated-Edge");
  if (graph->edges[graph->edgesCount].name == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas dodawania nazwy krawędzi!\n");
    return -1;
  }

  graph->edgesCount++;
  return 0;
}

/* Funkcja sprawdzająca spójność grafu za pomocą algorytmu DFS.
 * Jeśli graf jest niespójny, dodaje brakujące krawędzie, aby go połączyć. */
int ensureConnectivity(Graph *graph)
{
  bool repaired = false;
  if (graph->verticesCount == 0)
    return 1;

  bool *visited = calloc(graph->verticesCount, sizeof(bool));
  if (visited == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia tablicy algorytmu DFS!\n");
    return -1;
  }

  dfsVisit(graph, 0, visited);
  for (int i = 0; i < graph->verticesCount; i++)
  {
    if (visited[i] == false)
    {
      repaired = true;
      if (addExtraEdge(graph, 0, i) == -1)
      {
        free(visited);
        return -1;
      }
      dfsVisit(graph, i, visited);
    }
  }
  free(visited);
  return repaired ? 0 : 1;
}