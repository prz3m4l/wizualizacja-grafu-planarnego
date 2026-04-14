#include "planarity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  int *boundaryVertices;
  int boundaryCount;
} Face;

typedef struct
{
  int *vertices;
  int verticesCount;

  int *contactPoints;
  int contactCount;
  bool *inFragment;
  bool *isContact;

  int *admissibleFaces;
  int admissibleCount;
} Fragment;

/* Zwalnia pamięć dynamiczną zaalokowaną dla elementów w tablicy fragmentów. */
static void freeFragments(Fragment *fragments, int count)
{
  if (fragments == NULL)
    return;
  for (int i = 0; i < count; i++)
  {
    free(fragments[i].vertices);
    free(fragments[i].contactPoints);
    free(fragments[i].inFragment);
    free(fragments[i].isContact);
    free(fragments[i].admissibleFaces);
  }
  free(fragments);
}

/* Funkcja pomocnicza wykorzystująca DFS do znajdowania cyklu w grafie.
 * Zapisuje znaleziony cykl do tablicy 'cycles'. */
static bool dfsFindCycle(const Graph *graph, int currentVertex,
                         int parentVertex, int *parents, int *cycles,
                         int *cycleLength, bool *visited)
{
  visited[currentVertex] = true;
  parents[currentVertex] = parentVertex;

  for (int i = 0; i < (graph->vertices[currentVertex].count); i++)
  {
    int neighbor = graph->vertices[currentVertex].neighbours[i];

    if (visited[neighbor] == false)
    {
      if (dfsFindCycle(graph, neighbor, currentVertex, parents, cycles,
                       cycleLength, visited))
      {
        return true;
      }
    }
    else if (neighbor != parentVertex)
    {
      // Jeśli sąsiad jest odwiedzony i nie jest rodzicem - mamy cykl
      *cycleLength = 0;
      int curr = currentVertex;

      while (curr != neighbor && curr != -1)
      {
        cycles[*cycleLength] = curr;
        (*cycleLength)++;
        curr = parents[curr];
      }
      cycles[*cycleLength] = neighbor;
      (*cycleLength)++;

      return true;
    }
  }
  return false;
}

/* Zwraca indeks sąsiada 'v' na liście sąsiedztwa wierzchołka 'u'.
 * Jeśli 'v' nie jest sąsiadem 'u', zwraca -1. */
static int getNeighbourIndex(const Graph *graph, int u, int v)
{
  for (int i = 0; i < graph->vertices[u].count; i++)
  {
    if (graph->vertices[u].neighbours[i] == v)
      return i;
  }
  return -1;
}

/* Funkcja sprawdzająca planarność samego fragmentu (podgrafu) w izolacji.
 * Wyciąga fragment do osobnej struktury Graph i rekurencyjnie sprawdza isGraphPlanar. */
static bool checkFragmentPlanarity(const Graph *graph, Fragment *f)
{
  int subV = f->verticesCount + f->contactCount;
  if (subV < 3)
    return true;

  int *origToSub = malloc(graph->verticesCount * sizeof(int));
  if (origToSub == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas przypisywania origToSub!\n");
    return false;
  }
  for (int i = 0; i < graph->verticesCount; i++)
    origToSub[i] = -1;

  int currentSubId = 0;
  for (int i = 0; i < f->verticesCount; i++)
  {
    origToSub[f->vertices[i]] = currentSubId++;
  }
  for (int i = 0; i < f->contactCount; i++)
  {
    origToSub[f->contactPoints[i]] = currentSubId++;
  }

  Graph sub = {0};
  sub.verticesCount = subV;
  sub.vertices = calloc(subV, sizeof(Node));
  if (sub.vertices == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas generowania obiektu sub.vertices!\n");
    free(origToSub);
    return false;
  }

  for (int i = 0; i < f->verticesCount; i++)
  {
    int u = f->vertices[i];
    for (int j = 0; j < graph->vertices[u].count; j++)
    {
      int v = graph->vertices[u].neighbours[j];
      if (f->inFragment[v])
      {
        if (u < v)
        {
          if (addVertex(sub.vertices, origToSub[u], origToSub[v]) == -1 ||
              addVertex(sub.vertices, origToSub[v], origToSub[u]) == -1)
          {
            for (int k = 0; k < subV; k++)
              free(sub.vertices[k].neighbours);
            free(sub.vertices);
            free(origToSub);
            return false;
          }
          sub.edgesCount++;
        }
      }
      else if (f->isContact[v])
      {
        if (addVertex(sub.vertices, origToSub[u], origToSub[v]) == -1 ||
            addVertex(sub.vertices, origToSub[v], origToSub[u]) == -1)
        {
          for (int k = 0; k < subV; k++)
            free(sub.vertices[k].neighbours);
          free(sub.vertices);
          free(origToSub);
          return false;
        }
        sub.edgesCount++;
      }
    }
  }

  bool result = isGraphPlanar(&sub);

  for (int i = 0; i < subV; i++)
  {
    free(sub.vertices[i].neighbours);
  }
  free(sub.vertices);
  free(origToSub);

  return result;
}

/* Inicjalizuje pierwsze dwie ściany w algorytmie DMP, opierając się na znalezionym cyklu. */
static Face *initDmpFaces(int *cycle, int cycleLength, int verticesNum,
                          bool **vertexDrawnOut, int *facesCountOut,
                          int *maxFacesOut)
{
  bool *vertexDrawn = (bool *)calloc(verticesNum, sizeof(bool));
  if (vertexDrawn == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas inicjalizacji tablicy DMP!\n");
    return NULL;
  }

  for (int i = 0; i < cycleLength; i++)
  {
    vertexDrawn[cycle[i]] = true;
  }
  *vertexDrawnOut = vertexDrawn;

  int maxFaces = 2 * verticesNum > 4 ? 2 * verticesNum : 4;
  Face *faces = malloc(maxFaces * sizeof(Face));
  if (faces == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas przypisywania limitu ścian!\n");
    free(vertexDrawn);
    return NULL;
  }
  *maxFacesOut = maxFaces;

  faces[0].boundaryCount = cycleLength;
  faces[0].boundaryVertices = malloc(cycleLength * sizeof(int));
  if (faces[0].boundaryVertices == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas uzupełniania krawędzi ściany!\n");
    free(faces);
    free(vertexDrawn);
    return NULL;
  }
  for (int i = 0; i < cycleLength; i++)
    faces[0].boundaryVertices[i] = cycle[i];

  faces[1].boundaryCount = cycleLength;
  faces[1].boundaryVertices = malloc(cycleLength * sizeof(int));
  if (faces[1].boundaryVertices == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas finalizowania limitu ścian!\n");
    free(faces[0].boundaryVertices);
    free(faces);
    free(vertexDrawn);
    return NULL;
  }
  for (int i = 0; i < cycleLength; i++)
    faces[1].boundaryVertices[i] = cycle[i];

  *facesCountOut = 2;
  return faces;
}

/* Sprawdza, czy dany wierzchołek 'v' leży na obwodzie określonej ściany. */
static bool isVertexOnFace(int v, const Face *face)
{
  for (int i = 0; i < face->boundaryCount; i++)
  {
    if (face->boundaryVertices[i] == v)
      return true;
  }
  return false;
}

/* Sprawdza, w których ścianach (faces) fragment może zostać bezpiecznie narysowany. */
static bool findAdmissibleFaces(Fragment *frag, const Face *faces, int facesCount)
{
  frag->admissibleFaces = (int *)malloc(facesCount * sizeof(int));
  if (frag->admissibleFaces == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas zliczania dozwolonych ścian!\n");
    return false;
  }
  frag->admissibleCount = 0;

  for (int i = 0; i < facesCount; i++)
  {
    bool canBeDrawnHere = true;
    for (int j = 0; j < frag->contactCount; j++)
    {
      if (!isVertexOnFace(frag->contactPoints[j], &faces[i]))
      {
        canBeDrawnHere = false;
        break;
      }
    }
    if (canBeDrawnHere)
    {
      frag->admissibleFaces[frag->admissibleCount++] = i;
    }
  }
  return true;
}

/* Funkcja DFS rekurencyjnie budująca strukturę fragmentu z nienarysowanych krawędzi. */
static void dfsBuildFragment(const Graph *graph, int u, bool *vertexDrawn,
                             bool **edgeDrawn, int **edgeVisited, int visitId,
                             Fragment *frag)
{
  for (int i = 0; i < graph->vertices[u].count; i++)
  {
    int v = graph->vertices[u].neighbours[i];

    // Rozpatrujemy tylko nienarysowane i jeszcze nieodwiedzone w tym kroku
    // krawędzie
    if (!edgeDrawn[u][i] && edgeVisited[u][i] != visitId)
    {
      edgeVisited[u][i] = visitId;
      edgeVisited[v][getNeighbourIndex(graph, v, u)] = visitId;

      if (vertexDrawn[v])
      {
        if (!frag->isContact[v])
        {
          frag->isContact[v] = true;
          frag->contactPoints[frag->contactCount++] = v;
        }
      }
      else
      {
        if (!frag->inFragment[v])
        {
          frag->inFragment[v] = true;
          frag->vertices[frag->verticesCount++] = v;
          dfsBuildFragment(graph, v, vertexDrawn, edgeDrawn, edgeVisited, visitId,
                           frag);
        }
      }
    }
  }
}

/* Przeszukuje graf i znajduje wszystkie pozostałe nienarysowane fragmenty. */
static Fragment *getAllFragments(const Graph *graph, bool *vertexDrawn,
                                 bool **edgeDrawn, int **edgeVisited, int visitId, int *fragmentsCount)
{
  int V = graph->verticesCount;
  int maxFrags = graph->edgesCount > 0 ? graph->edgesCount : 1;
  Fragment *fragments = malloc(maxFrags * sizeof(Fragment));
  *fragmentsCount = 0;

  if (!fragments)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas inicjalizacji tablic fragmentów!\n");
    return NULL;
  }

  for (int u = 0; u < V; u++)
  {
    for (int i = 0; i < graph->vertices[u].count; i++)
    {
      int v = graph->vertices[u].neighbours[i];

      // Znaleźliśmy krawędź, która nie jest na płaszczyźnie i nie należy
      // jeszcze do żadnego fragmentu
      if (!edgeDrawn[u][i] && edgeVisited[u][i] != visitId)
      {
        Fragment *frag = &fragments[*fragmentsCount];

        frag->vertices = malloc(V * sizeof(int));
        frag->contactPoints = malloc(V * sizeof(int));
        frag->inFragment = calloc(V, sizeof(bool));
        frag->isContact = calloc(V, sizeof(bool));

        if (!frag->vertices || !frag->contactPoints || !frag->inFragment || !frag->isContact)
        {
          fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas alokacji elementów wewnętrznych fragmentu!\n");
          if (frag->vertices)
            free(frag->vertices);
          if (frag->contactPoints)
            free(frag->contactPoints);
          if (frag->inFragment)
            free(frag->inFragment);
          if (frag->isContact)
            free(frag->isContact);
          freeFragments(fragments, *fragmentsCount);
          free(fragments);
          return NULL;
        }

        frag->verticesCount = 0;
        frag->contactCount = 0;
        frag->admissibleFaces = NULL;
        frag->admissibleCount = 0;

        if (vertexDrawn[u] && vertexDrawn[v])
        {
          frag->contactPoints[frag->contactCount++] = u;
          frag->isContact[u] = true;
          frag->contactPoints[frag->contactCount++] = v;
          frag->isContact[v] = true;

          edgeVisited[u][i] = visitId;
          int idxV = getNeighbourIndex(graph, v, u);
          if (idxV != -1)
          {
            edgeVisited[v][idxV] = visitId;
          }
        }
        else
        {
          int startNode = vertexDrawn[u] ? v : u;

          if (vertexDrawn[u])
          {
            frag->contactPoints[frag->contactCount++] = u;
            frag->isContact[u] = true;
          }
          else if (vertexDrawn[v])
          {
            frag->contactPoints[frag->contactCount++] = v;
            frag->isContact[v] = true;
          }

          if (!vertexDrawn[startNode])
          {
            frag->vertices[frag->verticesCount++] = startNode;
            frag->inFragment[startNode] = true;
          }

          dfsBuildFragment(graph, startNode, vertexDrawn, edgeDrawn,
                           edgeVisited, visitId, frag);
        }

        (*fragmentsCount)++;
      }
    }
  }

  return fragments;
}

/* Szuka ścieżki wewnątrz danego fragmentu pomiędzy dwoma punktami kontaktowymi. */
static bool dfsFindPath(const Graph *graph, int current, int startNode,
                        Fragment *frag, bool **edgeDrawn, bool *visited,
                        int *tempPath, int depth, int **outPath,
                        int *outLength)
{
  visited[current] = true;
  tempPath[depth] = current;

  if (frag->isContact[current] && current != startNode)
  {
    *outLength = depth + 1;
    *outPath = malloc((*outLength) * sizeof(int));
    if (*outPath == NULL)
    {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas wyznaczania ścieżki w dfsFindPath!\n");
      return false;
    }
    for (int i = 0; i < *outLength; i++)
    {
      (*outPath)[i] = tempPath[i];
    }
    return true;
  }

  for (int i = 0; i < graph->vertices[current].count; i++)
  {
    int neighbor = graph->vertices[current].neighbours[i];

    // Upewniamy się, że nie przejdziemy po krawędzi już narysowanej na
    // płaszczyźnie
    if (!visited[neighbor] && !edgeDrawn[current][i])
    {
      if (frag->inFragment[neighbor] || frag->isContact[neighbor])
      {
        if (dfsFindPath(graph, neighbor, startNode, frag, edgeDrawn, visited,
                        tempPath, depth + 1, outPath, outLength))
        {
          return true;
        }
      }
    }
  }
  return false;
}

/* Określa i zwraca ścieżkę do narysowania w obrębie danego fragmentu. */
static int *findPathBetweenContacts(const Graph *graph, Fragment *frag,
                                    bool **edgeDrawn, int *pathLength)
{
  if (frag->verticesCount == 0 && frag->contactCount == 2)
  {
    *pathLength = 2;
    int *path = malloc(2 * sizeof(int));
    if (path == NULL)
    {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas zwrotu w findPathBetweenContacts!\n");
      return NULL;
    }
    path[0] = frag->contactPoints[0];
    path[1] = frag->contactPoints[1];
    return path;
  }

  if (frag->contactCount < 2)
  {
    *pathLength = 0;
    return NULL;
  }

  bool *visited = calloc(graph->verticesCount, sizeof(bool));
  int *tempPath = malloc(graph->verticesCount * sizeof(int));
  int *finalPath = NULL;
  *pathLength = 0;

  if (!visited || !tempPath)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas inicjalizacji tablic DFS!\n");
    free(visited);
    free(tempPath);
    *pathLength = 0;
    return NULL;
  }

  int startNode = frag->contactPoints[0];

  dfsFindPath(graph, startNode, startNode, frag, edgeDrawn, visited,
              tempPath, 0, &finalPath, pathLength);

  free(visited);
  free(tempPath);
  return finalPath;
}

/* Osadza ścieżkę wewnątrz ściany, co powoduje podzielenie ściany na dwie mniejsze. */
static bool embedPathAndSplitFace(Face **facesPtr, int *facesCount, int *maxFaces,
                                  int targetFaceIdx, int *path, int pathLength,
                                  bool *vertexDrawn, bool **edgeDrawn, const Graph *graph)
{
  Face *faces = *facesPtr;
  Face target = faces[targetFaceIdx];

  int idxStart = -1, idxEnd = -1;
  for (int i = 0; i < target.boundaryCount; i++)
  {
    if (target.boundaryVertices[i] == path[0])
      idxStart = i;
    if (target.boundaryVertices[i] == path[pathLength - 1])
      idxEnd = i;
  }

  int arc1_len =
      (idxEnd - idxStart + target.boundaryCount) % target.boundaryCount + 1;
  int arc2_len =
      (idxStart - idxEnd + target.boundaryCount) % target.boundaryCount + 1;

  int newFace1Count = arc1_len + pathLength - 2;
  int *newBoundary1 = malloc(newFace1Count * sizeof(int));
  if (newBoundary1 == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas osadzania ścieżki w ścianie 1!\n");
    return false;
  }
  int c1 = 0;
  for (int i = 0; i < arc1_len; i++)
  {
    newBoundary1[c1++] =
        target.boundaryVertices[(idxStart + i) % target.boundaryCount];
  }
  for (int i = pathLength - 2; i >= 1; i--)
  {
    newBoundary1[c1++] = path[i];
  }

  int newFace2Count = arc2_len + pathLength - 2;
  int *newBoundary2 = malloc(newFace2Count * sizeof(int));
  if (newBoundary2 == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas osadzania ścieżki w ścianie 2!\n");
    free(newBoundary1);
    return false;
  }
  int c2 = 0;
  for (int i = 0; i < arc2_len; i++)
  {
    newBoundary2[c2++] =
        target.boundaryVertices[(idxEnd + i) % target.boundaryCount];
  }
  for (int i = 1; i < pathLength - 1; i++)
  {
    newBoundary2[c2++] = path[i];
  }

  if (*facesCount >= *maxFaces)
  {
    int newMaxFaces = (*maxFaces) * 2;
    Face *tmp = realloc(*facesPtr, newMaxFaces * sizeof(Face));
    if (tmp == NULL)
    {
      fprintf(stderr, "Błąd! Nie można powiększyć tablicy ścian przez realloc!\n");
      free(newBoundary1);
      free(newBoundary2);
      return false;
    }
    *facesPtr = tmp;
    faces = *facesPtr;
    *maxFaces = newMaxFaces;
  }

  free(target.boundaryVertices);
  faces[targetFaceIdx].boundaryVertices = newBoundary1;
  faces[targetFaceIdx].boundaryCount = newFace1Count;

  faces[*facesCount].boundaryVertices = newBoundary2;
  faces[*facesCount].boundaryCount = newFace2Count;
  (*facesCount)++;

  // Oznaczamy i wierzchołki i krawedzie jako narysowane
  for (int i = 1; i < pathLength - 1; i++)
  {
    vertexDrawn[path[i]] = true;
  }
  for (int i = 0; i < pathLength - 1; i++)
  {
    int u = path[i];
    int v = path[i + 1];
    int idxU = getNeighbourIndex(graph, u, v);
    if (idxU != -1)
      edgeDrawn[u][idxU] = true;
    int idxV = getNeighbourIndex(graph, v, u);
    if (idxV != -1)
      edgeDrawn[v][idxV] = true;
  }
  return true;
}

/* Główna funkcja algorytmu sprawdzającego planarność grafu (Algorytm DMP).
 * Zwraca true, jeśli graf można narysować na płaszczyźnie bez przecięć krawędzi. */
bool isGraphPlanar(Graph *graph)
{
  int V = graph->verticesCount;
  if (V >= 3)
  {
    if (graph->edgesCount > 3 * V - 6)
    {
      return false;
    }
  }
  else
  {
    return true;
  }

  int *parents = malloc(V * sizeof(int));
  int *cycles = malloc(V * sizeof(int));
  bool *visited = calloc(V, sizeof(bool));
  int cycleLength = 0;

  if (!parents || !cycles || !visited)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas określania cykli roboczych!\n");
    free(parents);
    free(cycles);
    free(visited);
    return false;
  }

  bool hasCycle = false;
  for (int i = 0; i < V; i++)
  {
    if (!visited[i])
    {
      if (dfsFindCycle(graph, i, -1, parents, cycles, &cycleLength, visited))
      {
        hasCycle = true;
        break;
      }
    }
  }

  if (!hasCycle)
  {
    free(parents);
    free(cycles);
    free(visited);
    return true;
  }

  bool *vertexDrawn = NULL;
  int facesCount = 0;
  int maxFaces = 0;
  Face *faces =
      initDmpFaces(cycles, cycleLength, V, &vertexDrawn, &facesCount, &maxFaces);

  if (faces == NULL)
  {
    free(parents);
    free(cycles);
    free(visited);
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas inicjalizacji tablic DMP!\n");
    return false;
  }

  bool **edgeDrawn = malloc(V * sizeof(bool *));
  int **edgeVisited = malloc(V * sizeof(int *));
  if (!edgeDrawn || !edgeVisited)
  {
    free(parents);
    free(cycles);
    free(visited);
    free(vertexDrawn);
    for (int k = 0; k < facesCount; k++)
    {
      free(faces[k].boundaryVertices);
    }
    free(faces);
    free(edgeDrawn);
    free(edgeVisited);
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas generowania macierzy sprawdzania krawędzi!\n");
    return false;
  }
  for (int i = 0; i < V; i++)
  {
    edgeDrawn[i] = calloc(graph->vertices[i].count, sizeof(bool));
    edgeVisited[i] = calloc(graph->vertices[i].count, sizeof(int));

    if (!edgeDrawn[i] || !edgeVisited[i])
    {
      for (int j = 0; j <= i; j++)
      {
        free(edgeDrawn[j]);
        free(edgeVisited[j]);
      }
      free(edgeDrawn);
      free(edgeVisited);
      free(parents);
      free(cycles);
      free(visited);
      free(vertexDrawn);
      for (int k = 0; k < facesCount; k++)
      {
        free(faces[k].boundaryVertices);
      }
      free(faces);
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas wypełniania macierzy krawędzi!\n");
      return false;
    }
  }
  int visitId = 0;

  // Oznaczamy krawędzie z głównego cyklu jako narysowane
  for (int i = 0; i < cycleLength; i++)
  {
    int u = cycles[i];
    int v = cycles[(i + 1) % cycleLength];
    int idxU = getNeighbourIndex(graph, u, v);
    if (idxU != -1)
      edgeDrawn[u][idxU] = true;
    int idxV = getNeighbourIndex(graph, v, u);
    if (idxV != -1)
      edgeDrawn[v][idxV] = true;
  }

  bool isPlanar = true;

  while (true)
  {
    visitId++;
    int fragmentsCount = 0;
    Fragment *fragments =
        getAllFragments(graph, vertexDrawn, edgeDrawn, edgeVisited, visitId, &fragmentsCount);

    if (fragmentsCount == 0)
    {
      free(fragments);
      break;
    }

    int chosenFragmentIdx = -1;
    int targetFaceIdx = -1;

    for (int i = 0; i < fragmentsCount; i++)
    {
      if (!findAdmissibleFaces(&fragments[i], faces, facesCount))
      {
        isPlanar = false;
        break;
      }

      if (fragments[i].admissibleCount == 0)
      {
        isPlanar = false;
        break;
      }
      if (fragments[i].admissibleCount == 1)
      {
        chosenFragmentIdx = i;
        targetFaceIdx = fragments[i].admissibleFaces[0];
      }
    }

    if (!isPlanar)
    {
      freeFragments(fragments, fragmentsCount);
      break;
    }

    if (chosenFragmentIdx == -1)
    {
      chosenFragmentIdx = 0;
      targetFaceIdx = fragments[0].admissibleFaces[0];
    }

    int pathLength = 0;
    int *path = findPathBetweenContacts(graph, &fragments[chosenFragmentIdx],
                                        edgeDrawn, &pathLength);
    if (path == NULL)
    {
      Fragment *f = &fragments[chosenFragmentIdx];

      if (!checkFragmentPlanarity(graph, f))
      {
        isPlanar = false;
        freeFragments(fragments, fragmentsCount);
        break;
      }

      // Przechodzimy przez wszystkie wierzchołki ogona
      for (int k = 0; k < f->verticesCount; k++)
      {
        int u = f->vertices[k];

        // Oznaczamy wierzchołek jako narysowany
        vertexDrawn[u] = true;

        // Oznaczamy wszystkie krawędzie wewnątrz tego ogona jako narysowane
        for (int j = 0; j < graph->vertices[u].count; j++)
        {
          int v = graph->vertices[u].neighbours[j];
          if (f->inFragment[v] || f->isContact[v])
          {
            edgeDrawn[u][j] = true;
            int idxV = getNeighbourIndex(graph, v, u);
            if (idxV != -1)
              edgeDrawn[v][idxV] = true;
          }
        }
      }

      // Zwalniamy pamięć z tej iteracji i przeskakujemy do kolejnej
      freeFragments(fragments, fragmentsCount);
      continue;
    }

    if (!embedPathAndSplitFace(&faces, &facesCount, &maxFaces, targetFaceIdx, path,
                               pathLength, vertexDrawn, edgeDrawn, graph))
    {
      isPlanar = false;
      free(path);
      freeFragments(fragments, fragmentsCount);
      break;
    }

    free(path);
    freeFragments(fragments, fragmentsCount);
  }

  free(parents);
  free(cycles);
  free(visited);
  free(vertexDrawn);
  for (int i = 0; i < V; i++)
  {
    free(edgeDrawn[i]);
    free(edgeVisited[i]);
  }
  free(edgeDrawn);
  free(edgeVisited);
  for (int i = 0; i < facesCount; i++)
    free(faces[i].boundaryVertices);
  free(faces);

  return isPlanar;
}

/* Funkcja weryfikująca planarność. Usuwa problematyczne krawędzie jeśli graf
 * nie jest planarny, zwracając liczbę usuniętych krawędzi. */
int makeGraphPlanar(Graph *graph)
{
  if (isGraphPlanar(graph))
  {
    return 0;
  }

  fprintf(stderr, "Ostrzeżenie: Graf nieplanarny! Rozpoczynam naprawę (usuwanie krawędzi)...\n");

  int origEdgesCount = graph->edgesCount;
  Edge *origEdges = graph->edges;

  graph->edges = malloc(origEdgesCount * sizeof(Edge));
  if (graph->edges == NULL)
  {
    fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas naprawy planarności!\n");
    graph->edges = origEdges;
    return -1;
  }
  graph->edgesCount = 0;

  for (int i = 0; i < graph->verticesCount; i++)
  {
    graph->vertices[i].count = 0;
  }

  /* Tarcza Eulera: graf planarny ma co najwyżej 3v-6 krawędzi */
  int maxEdgesLimit = (graph->verticesCount >= 3) ? (3 * graph->verticesCount - 6) : origEdgesCount;

  int removedEdges = 0;

  for (int i = 0; i < origEdgesCount; i++)
  {
    Edge currentEdge = origEdges[i];

    /* Tarcza Eulera: odrzuć bez wywołania isGraphPlanar */
    if (graph->edgesCount == maxEdgesLimit)
    {
      fprintf(stderr, "Ostrzeżenie: Odrzucono krawędź \"%s\" (%d - %d) aby zapewnić planarność.\n",
              currentEdge.name, currentEdge.idA, currentEdge.idB);
      free(currentEdge.name);
      removedEdges++;
      continue;
    }

    graph->edges[graph->edgesCount] = currentEdge;
    graph->edgesCount++;
    if (addVertex(graph->vertices, currentEdge.idA, currentEdge.idB) == -1 ||
        addVertex(graph->vertices, currentEdge.idB, currentEdge.idA) == -1)
    {
      fprintf(stderr, "Błąd! Nie można zaalokować pamięci podczas tworzenia bezpiecznej kopii krawędzi!\n");
      for (int k = i + 1; k < origEdgesCount; k++)
      {
        free(origEdges[k].name);
      }
      free(origEdges);
      return -1;
    }

    if (!isGraphPlanar(graph))
    {
      graph->edgesCount--;

      Node *na = &graph->vertices[currentEdge.idA];
      for (int j = 0; j < na->count; j++)
      {
        if (na->neighbours[j] == currentEdge.idB)
        {
          na->neighbours[j] = na->neighbours[na->count - 1];
          na->count--;
          break;
        }
      }

      Node *nb = &graph->vertices[currentEdge.idB];
      for (int j = 0; j < nb->count; j++)
      {
        if (nb->neighbours[j] == currentEdge.idA)
        {
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