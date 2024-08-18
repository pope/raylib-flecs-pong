PKGS = raylib

OPT_FLAGS = -O3 -flto=auto -m64 -march=native -mtune=native

CFLAGS += -g -Wall -Wextra -pedantic -std=c++20
CFLAGS += -Isrc -Ivendor
CFLAGS += $(shell pkg-config --cflags $(PKGS))

LDFLAGS += $(shell pkg-config --libs $(PKGS))

SRCS = $(wildcard src/*.cc)
DEPS = $(wildcard src/*.h)

DEBUG_OBJS = $(patsubst src/%.cc, build/debug/%.o, $(SRCS))
RELEASE_OBJS = $(patsubst src/%.cc, build/release/%.o, $(SRCS))

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
	$(CXX) $(LDFLAGS) $^ -o $@

$(RELEASE_EXE): $(RELEASE_OBJS) $(FLECS_OBJS)
	$(CXX) $(LDFLAGS) $(OPT_FLAGS) $^ -o $@

build/debug/%.o: src/%.cc $(DEPS) Makefile | build
	$(CXX) -c $(DEBUG_CFLAGS) $< -o $@

build/release/%.o: src/%.cc $(DEPS) Makefile | build
	$(CXX) -c $(RELEASE_CFLAGS) $< -o $@

build/flecs/flecs.o: vendor/flecs.c Makefile | build
	$(CC) -c $(FLECS_CFLAGS) $< -o $@

build:
	-mkdir -p build/{debug,flecs,release}

clean:
	-rm -rf build

.SUFFIXES:
MAKEFLAGS += --no-builtin-rules
.PHONY: debug release clean run run_release
