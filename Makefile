CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -O2 -std=c11
LDFLAGS = -lm
EXEC    = wizualizacja_grafu

SRCS = src/main.c src/graph.c src/planarity.c src/io_manager.c \
       src/fr_algorithm.c src/kk_algorithm.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC) -i data/graf.txt -o data/wyniki.txt -a kamada -s 17

kkss: $(EXEC)
	./$(EXEC) -i data/graf.txt -o data/wyniki.txt -a kamada -s 1

frss: $(EXEC)
	./$(EXEC) -i data/graf.txt -o data/wyniki.txt -a fruchterman -s 1

kkrs: $(EXEC)
	./$(EXEC) -i data/graf.txt -o data/wyniki.txt -a kamada

frrs: $(EXEC)
	./$(EXEC) -i data/graf.txt -o data/wyniki.txt -a fruchterman