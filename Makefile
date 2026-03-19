CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2 -std=c11
LDFLAGS = -lm

SRCS = src/main.c src/graph.c src/algorithms.c src/io_manager.c
EXEC = wizualizacja_grafu

.PHONY: all clean run

all: $(EXEC)

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXEC) $(LDFLAGS)

clean:
	rm -f $(EXEC)

run: $(EXEC)
	./$(EXEC) -i data/graf.txt -o data/wyniki.txt -a kamada -s 17