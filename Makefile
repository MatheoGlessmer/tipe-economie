# Simulation économique (TIPE 2026)
#
# make            compile le programme de simulation       -> build/simulation
# make analysis   compile l'outil d'analyse des matrices   -> build/matrix_analysis
# make run        compile et lance la suite de tests       -> build/eco_tests
# make no-color   idem, sans couleurs
# make clean      supprime build/
#
# Les trois programmes sont indépendants : matrix_analysis ne fait pas partie
# de la simulation, c'est un outil qui lit les matrices (.mat) produites.
#
# Pour renommer 0_include / 1_src en include / src, il suffit de changer
# INC_DIR et SRC_DIR ci-dessous (après avoir fait les git mv).

CC       ?= gcc
CFLAGS   ?= -std=c99 -Wall -Wextra -pedantic -Wno-address -g
LDFLAGS  ?=
LDLIBS   ?= -lm

INC_DIR   := 0_include
SRC_DIR   := 1_src
BUILD_DIR := build

# Les en-têtes sont rangés par rôle : un -I par sous-dossier, donc les
# #include "economie_model.h" existants continuent de fonctionner sans changement.
CPPFLAGS ?= -I$(INC_DIR)/main -I$(INC_DIR)/utils -I$(INC_DIR)/tests

HEADERS    := $(wildcard $(INC_DIR)/*/*.h)
CORE_SRCS  := $(wildcard $(SRC_DIR)/main/*.c)
UTILS_SRCS := $(wildcard $(SRC_DIR)/utils/*.c)
TEST_SRCS  := $(wildcard $(SRC_DIR)/tests/*.c)

# --- Programme de simulation -------------------------------------------------
SIM_BIN  := $(BUILD_DIR)/simulation
SIM_SRCS := $(SRC_DIR)/main.c $(CORE_SRCS) $(UTILS_SRCS)

# --- Outil d'analyse des matrices (hors simulation) --------------------------
# Si tu déplaces matrix_analysis.c (par exemple dans 1_src/tools/), change
# ANALYSIS_MAIN. Si l'outil a besoin de market.c ou de jacobien.c, ajoute
# $(CORE_SRCS) à ANALYSIS_SRCS.
ANALYSIS_BIN  := $(BUILD_DIR)/matrix_analysis
ANALYSIS_MAIN := $(SRC_DIR)/matrix_analysis.c
ANALYSIS_SRCS := $(ANALYSIS_MAIN) $(UTILS_SRCS)
# LAPACKE (valeurs propres via LAPACKE_dgeev). Sur Debian/Ubuntu/Mint :
#   sudo apt install liblapacke-dev
ANALYSIS_LIBS := -llapacke -llapack -lblas -lm

# --- Suite de tests ----------------------------------------------------------
# Les fonctions non fournies sont marquées TODO grâce aux symboles faibles.
TEST_BIN  := $(BUILD_DIR)/eco_tests
IMPL_SRCS ?= $(SRC_DIR)/utils/economie_model.c $(SRC_DIR)/utils/economie_display.c

.PHONY: all analysis run no-color clean

all: $(SIM_BIN)

analysis: $(ANALYSIS_BIN)

$(SIM_BIN): $(SIM_SRCS) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SIM_SRCS) $(LDFLAGS) $(LDLIBS) -o $@

$(ANALYSIS_BIN): $(ANALYSIS_SRCS) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(ANALYSIS_SRCS) $(LDFLAGS) $(ANALYSIS_LIBS) -o $@

$(TEST_BIN): $(TEST_SRCS) $(IMPL_SRCS) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TEST_SRCS) $(IMPL_SRCS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TEST_BIN)
	./$(TEST_BIN)

no-color: $(TEST_BIN)
	./$(TEST_BIN) --no-color

clean:
	rm -rf $(BUILD_DIR)