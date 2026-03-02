CC = gcc
CFLAGS = -Wall -Wextra -pedantic

SRCS = src/main.c src/graph.c src/algorithms.c src/io_manager.c

EXEC = wizualizacja_grafu

all: $(EXEC)

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXEC) -lm

clean:
	rm -f $(EXEC) *.o