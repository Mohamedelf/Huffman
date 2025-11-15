# Makefile pour huffman-project
# Usage rapide :
#   make           -> compile en release (optimisé)
#   make debug     -> compile en debug (-g, -O0)
#   make run ARGS="..."     -> compile puis exécute ./huffman $(ARGS)
#   make valgrind ARGS="..."-> exécute sous valgrind
#   make clean     -> supprime build/ et exécutable
#   make help      -> affiche l'aide

# ----------------- Configuration -----------------
CC       := gcc
CFLAGS   := -Wall -Wextra -std=c11 -O2
DEBUG_FLAGS := -g -O0 -DDEBUG
LDFLAGS  :=

SRC_DIR  := src
BUILD_DIR:= build
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS     := $(OBJS:.o=.d)
TARGET   := huffman

# Arguments utilisateur (ex: make run ARGS="-c in out")
ARGS     ?=

# ----------------- Règles principales -----------------
.PHONY: all debug clean run valgrind help

all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Compilation des .c en .o (avec génération de dépendances .d)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "[CC] $<"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Crée le répertoire build si nécessaire
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# ----------------- Modes -----------------
# Mode debug : ajoute options de debug et recompile
debug:
	@$(MAKE) clean
	@$(MAKE) CFLAGS="$(CFLAGS) $(DEBUG_FLAGS)" all

# Exécution (après compilation)
run: all
	@echo "[RUN] ./$(TARGET) $(ARGS)"
	@./$(TARGET) $(ARGS)

# Lance sous valgrind (nécessite valgrind installé)
valgrind: all
	@echo "[VALGRIND] ./$(TARGET) $(ARGS)"
	@valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) $(ARGS)

# Nettoyage des fichiers compilés
clean:
	@echo "[CLEAN] remove build/ and $(TARGET)"
	@rm -rf $(BUILD_DIR) $(TARGET) $(DEPS)

# Aide
help:
	@printf "Makefile targets:\n"
	@printf "  make         : build release (CFLAGS=%s)\n" "$(CFLAGS)"
	@printf "  make debug   : clean + build debug (CFLAGS += %s)\n" "$(DEBUG_FLAGS)"
	@printf "  make run ARGS=\"...\"      : build then run with ARGS\n"
	@printf "  make valgrind ARGS=\"...\" : build then run under valgrind\n"
	@printf "  make clean   : remove build artifacts\n"
	@printf "  make help    : show this message\n"

# ----------------- Include des dépendances automatiques (si présents) --
-include $(DEPS)
