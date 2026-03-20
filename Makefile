CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2

SRCS = src/main.c src/graph.c src/algorithms.c src/io_manager.c
EXEC = wizualizacja_grafu

.PHONY: all clean run run1 run2 run3

all: $(EXEC)

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXEC) -lm

clean:
	rm -f $(EXEC) wyniki*.txt

# Podstawowy run (to samo co run1)
run: run1

# Test 1: Prosty graf planarny (używa Fruchtermana)
run1: $(EXEC)
	./$(EXEC) -i data/graf1.txt -o data/wyniki1.txt -a fruchterman

# Test 2: Graf niespójny (używa Kamady)
run2: $(EXEC)
	./$(EXEC) -i data/graf2.txt -o data/wyniki2.txt -a kamada

# Test 3: Graf nieplanarny K5 (Wymusi auto-naprawę)
run3: $(EXEC)
	./$(EXEC) -i data/graf3.txt -o data/wyniki3.txt -a fruchterman