PKGS = raylib

UNAME_S := $(shell uname -s)

OPT_FLAGS = -O3
ifeq ($(UNAME_S),Linux)
	OPT_FLAGS += -flto=auto -m64 -march=native -mtune=native
endif

CFLAGS += -g -Wall -Wextra -pedantic -std=c99 
CFLAGS += -Isrc -Ivendor
CFLAGS += $(shell pkg-config --cflags $(PKGS))

LDFLAGS += $(shell pkg-config --libs $(PKGS))

SRCS = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)

DEBUG_OBJS = $(patsubst src/%.c, build/debug/%.o, $(SRCS))
RELEASE_OBJS = $(patsubst src/%.c, build/release/%.o, $(SRCS))

DEBUG_EXE = build/debug/main
RELEASE_EXE = build/release/main

DEBUG_CFLAGS += $(CFLAGS) -Og

RELEASE_CFLAGS += $(CFLAGS)
RELEASE_CFLAGS += -DNDEBUG $(OPT_FLAGS)

FLECS_OBJS += build/flecs/flecs.o
FLECS_CFLAGS += -std=gnu99 $(OPT_FLAGS)

debug: $(DEBUG_EXE)
release: $(RELEASE_EXE)

run: $(DEBUG_EXE)
	./$<

run_release: $(RELEASE_EXE)
	./$<

$(DEBUG_EXE): $(DEBUG_OBJS) $(FLECS_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(RELEASE_EXE): $(RELEASE_OBJS) $(FLECS_OBJS)
	$(CC) $(LDFLAGS) $(OPT_FLAGS) $^ -o $@

build/debug/%.o: src/%.c $(DEPS) Makefile | build
	$(CC) -c $(DEBUG_CFLAGS) $< -o $@

build/release/%.o: src/%.c $(DEPS) Makefile | build
	$(CC) -c $(RELEASE_CFLAGS) $< -o $@

build/flecs/flecs.o: vendor/flecs.c Makefile | build
	$(CC) -c $(FLECS_CFLAGS) $< -o $@

build:
	-mkdir -p build/{debug,flecs,release}

clean:
	-rm -rf build

.SUFFIXES:
MAKEFLAGS += --no-builtin-rules
.PHONY: debug release clean run run_release
