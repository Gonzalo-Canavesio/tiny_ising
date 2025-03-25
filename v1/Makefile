CC=gcc
CFLAGS=-std=c11 -Wall -Wextra
EXTRAFLAGS=-lgomp -lm
GL_LDFLAGS=-lGL -lglfw

# Files
TARGETS=tiny_ising

# Rules
all: $(TARGETS)

tiny_ising: tiny_ising.o ising.o
	$(CC) $(CFLAGS) -o $@ $^ $(EXTRAFLAGS)

clean:
	rm -f $(TARGETS) *.o

.PHONY: clean all
