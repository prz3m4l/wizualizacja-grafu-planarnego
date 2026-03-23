# Wizualizacja grafu planarnego

Program wyznacza dwuwymiarowe współrzędne wierzchołków grafu planarnego
na podstawie listy krawędzi.

## Kompilacja
```bash
make
```

## Użycie
```bash
./wizualizacja_grafu -i <plik_wejściowy> {-o|-b} <plik_wyjściowy> [opcje]
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
| `-s <liczba>` | Ziarno generatora losowego | czas systemowy |

\*Wymagana jedna z dwóch: `-o` lub `-b`

## Przykład
```bash
./wizualizacja_grafu -i graf.txt -o wyniki.txt -a kamada -t 500 -s 42
```

### Format pliku wejściowego
```
# komentarz
AB  1  2  1.0
BC  2  3  1.0
CD  3  4  1.0
```

Każda linia opisuje jedną krawędź w formacie: `<nazwa> <wierzchołek_A> <wierzchołek_B> <waga>`.
Linie zaczynające się od `#` są traktowane jako komentarze.

### Format pliku wyjściowego (tekstowy)
```
1 0.0 0.0
2 1.0 0.0
3 1.0 1.0
```

### Format pliku wyjściowego (binarny)
Dla każdego wierzchołka kolejno: długość nazwy (`uint32_t`, big-endian), nazwa (`char[]`), współrzędna x (`double`, big-endian), współrzędna y (`double`, big-endian).

## Algorytmy

- **Fruchterman-Reingold** — symulacja sił odpychania i przyciągania między wierzchołkami
- **Kamada-Kawai** — minimalizacja energii sprężyn, lepsze odwzorowanie odległości w grafie

## Uwagi

- Graf jest automatycznie sprowadzany do planarnego (nadmiarowe krawędzie są usuwane).
- Jeśli graf jest niespójny, program automatycznie dodaje brakujące krawędzie i wyświetla ostrzeżenie.
- Pętle własne (krawędź łącząca wierzchołek z samym sobą) są wykrywane i zgłaszane jako ostrzeżenie.