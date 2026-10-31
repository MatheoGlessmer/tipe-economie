CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -Wno-address -g
CPPFLAGS ?= -Iinclude -Isrc
LDFLAGS ?=
LDLIBS ?=

# Les deux fichiers sources de l'exemple sont maintenant inclus par défaut
IMPL_SRCS ?= code/economie_model.c code/economie_display.c

TEST_BIN := build/eco_tests
TEST_SRCS := \
    src/eco_test_framework.c \
    src/eco_test_utils.c \
    tests/main.c \
    tests/test_structure_helpers.c \
    tests/test_model_alloc.c \
    tests/test_model_algorithms.c \
    tests/test_display.c

.PHONY: all run clean no-color

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_SRCS) $(IMPL_SRCS) | build
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TEST_SRCS) $(IMPL_SRCS) $(LDFLAGS) $(LDLIBS) -o $@

build:
	mkdir -p build

run: $(TEST_BIN)
	./$(TEST_BIN)

no-color: $(TEST_BIN)
	./$(TEST_BIN) --no-color

clean:
	rm -rf build