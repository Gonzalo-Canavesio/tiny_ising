CC=gcc
CFLAGS=-std=c11 -Wall -Wextra
EXTRAFLAGS=-lgomp -lm
GL_LDFLAGS=-lGL -lglfw

# Files
TARGETS=tiny_ising demo

# Rules
all: $(TARGETS)

tiny_ising: tiny_ising.o ising.o xoshiro256plus.o
	$(CC) $(CFLAGS) -o $@ $^ $(EXTRAFLAGS)

demo: demo.o ising.o xoshiro256plus.o
	$(CC) $(CFLAGS) -o $@ $^ $(EXTRAFLAGS) $(GL_LDFLAGS)

clean:
	rm -f $(TARGETS) *.o

.PHONY: clean all

