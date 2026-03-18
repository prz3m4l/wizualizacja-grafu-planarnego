# Wizualizacja grafu planarnego

Program wyznacza dwuwymiarowe współrzędne wierzchołków grafu planarnego
na podstawie listy krawędzi.
## Kompilacja
```bash
make
```

Lub ręcznie:
```bash
gcc -O2 -Wall -Wextra -o graph_layout \
    main.c graph.c io_manager.c algorithms.c -lm
```

## Użycie
```bash
./graph_layout -i <plik_wejściowy> {-o|-b} <plik_wyjściowy> [opcje]
```

### Flagi

| Flaga | Opis | Domyślnie |
|-------|------|-----------|
| `-i <ścieżka>` | Plik wejściowy z listą krawędzi | wymagana |
| `-o <ścieżka>` | Plik wyjściowy (format tekstowy) | wymagana\* |
| `-b <ścieżka>` | Plik wyjściowy (format binarny) | wymagana\* |
| `-a <nazwa>` | Algorytm: `fruchterman` lub `kamada` | `fruchterman` |
| `-w <liczba>` | Szerokość obszaru roboczego | `1000` |
| `-h <liczba>` | Wysokość obszaru roboczego | `1000` |
| `-t <liczba>` | Liczba iteracji | `100` |

\*Wymagana jedna z dwóch: `-o` lub `-b`

## Przykład
```bash
./graph_layout -i graf.txt -o wyniki.txt -a kamada -t 500
```

### Format pliku wejściowego
```
# komentarz
AB  1  2  1.0
BC  2  3  1.0
CD  3  4  1.0
```

### Format pliku wyjściowego
```
1 0.0 0.0
2 1.0 0.0
3 1.0 1.0
```

## Algorytmy

- **Fruchterman-Reingold** — symulacja sił odpychania i przyciągania
- **Kamada-Kawai** — minimalizacja energii sprężyn, lepsze odwzorowanie odległości w grafie