.PHONY: all clean others


PKGS = raylib

CFLAGS += -g -Wall -Wextra -pedantic -std=c99 -Ivendor
CFLAGS += $(shell pkg-config --cflags $(PKGS))

LDFLAGS += $(shell pkg-config --libs $(PKGS))

# CFLAGS += -DNDEBUG -O3

all: main

foo.c: foo.h
input.c: input.h
rendering.c: rendering.h
main: main.c foo.c input.c rendering.c vendor/flecs.o

vendor/flecs.o:
	$(MAKE) -C vendor

clean:
	-rm main
