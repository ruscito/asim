CC=cc
CFLAGS=-O2 -Wall -Wextra -std=c99
LDFLAGS=

.PHONY: all clean

all: clean asim

sim: sim.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

sim.o: sim.c sim.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o asim
