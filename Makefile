# mach unified build system
# builds:
#   1. bootstrap c compiler (boot/ -> out/bin/cmach)
#   2. intermediary compiler with cmach (src/ -> out/bin/imach)
#   3. final compiler with imach (src/ -> out/bin/mach)
#
# directory structure:
#   out/
#     bin/         # final binaries (cmach, imach, mach)
#     cmach/       # cmach artifacts (obj/)
#     imach/<target>/  # imach artifacts (ast/, ir/, asm/, obj/)
#     mach/<target>/   # mach artifacts (ast/, ir/, asm/, obj/)

# compiler and flags
CC := clang
CFLAGS := -std=c23 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2
CFLAGS_DEBUG := -std=c23 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -g -O0 -DDEBUG

OUT_DIR := out
BIN_DIR := $(OUT_DIR)/bin

# standard library (source only, referenced directly)
STD_DIR := std
MACH_SRC_DIR := src

# bootstrap compiler
BOOT_DIR := boot
BOOT_SRC_DIR := $(BOOT_DIR)/src
BOOT_INC_DIR := $(BOOT_DIR)/include
CMACH_OBJ_DIR := $(OUT_DIR)/cmach/obj

# final executables
CMACH := $(BIN_DIR)/cmach
IMACH := $(BIN_DIR)/imach
MACH := $(BIN_DIR)/mach

# bootstrap compiler sources
BOOT_SOURCES := $(shell find $(BOOT_SRC_DIR) -type f -name '*.c')
BOOT_OBJECTS := $(BOOT_SOURCES:$(BOOT_SRC_DIR)/%.c=$(CMACH_OBJ_DIR)/%.o)
BOOT_HEADERS := $(shell find $(BOOT_INC_DIR) -type f -name '*.h')

# main targets
.PHONY: help cmach-clean cmach-build cmach imach-clean imach-build imach mach-clean mach-build mach full clean

# default target: print help
.DEFAULT_GOAL := help

help:
	@echo "mach compiler build system"
	@echo ""
	@echo "bootstrap (c compiler):"
	@echo "  cmach-clean  - clean cmach build artifacts"
	@echo "  cmach-build  - build cmach"
	@echo "  cmach        - clean and build cmach"
	@echo ""
	@echo "intermediary (compiled with cmach):"
	@echo "  imach-clean  - clean imach build artifacts"
	@echo "  imach-build  - build imach"
	@echo "  imach        - clean and build imach"
	@echo ""
	@echo "final (compiled with imach):"
	@echo "  mach-clean   - clean mach build artifacts"
	@echo "  mach-build   - build mach"
	@echo "  mach         - clean and build mach"
	@echo ""
	@echo "meta:"
	@echo "  clean        - clean all build artifacts"
	@echo ""
	@echo "note: target platform determined by mach.toml (target = \"native\")"
	@echo "  imach        - clean and build imach"
	@echo ""
	@echo "final (compiled with imach):"
	@echo "  mach-clean   - clean mach build artifacts"
	@echo "  mach-build   - build mach"
	@echo "  mach         - clean and build mach"
	@echo ""
	@echo "meta:"
	@echo "  clean        - clean all build artifacts"

# bootstrap compiler
cmach-clean:
	@echo "cleaning cmach"
	@rm -rf $(OUT_DIR)/cmach
	@rm -f $(CMACH)

cmach-build: $(CMACH)

cmach: cmach-clean cmach-build

# intermediary compiler
imach-clean:
	@echo "cleaning imach"
	@rm -rf $(OUT_DIR)/imach
	@rm -f $(IMACH)

imach-build: $(IMACH)

imach: imach-clean imach-build

# final compiler
mach-clean:
	@echo "cleaning mach"
	@rm -rf $(OUT_DIR)/mach
	@rm -f $(MACH)

mach-build: $(MACH)

mach: mach-clean mach-build

# clean everything
clean:
	@echo "cleaning all"
	@rm -rf $(OUT_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(CMACH_OBJ_DIR):
	@mkdir -p $(CMACH_OBJ_DIR)

$(CMACH_OBJ_DIR)/%.o: $(BOOT_SRC_DIR)/%.c $(BOOT_HEADERS) | $(CMACH_OBJ_DIR)
	@echo "  cc  $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -I$(BOOT_INC_DIR) -c $< -o $@

# bootstrap compiler build
$(CMACH): $(BOOT_OBJECTS) | $(BIN_DIR)
	@echo "  ld  $@"
	@$(CC) $(BOOT_OBJECTS) -o $@
	@echo "bootstrap compiler ready: $@"

# intermediary compiler build (using cmach)
# mach.toml determines target and output paths
$(IMACH): $(CMACH)
	@echo "  cleaning artifacts for imach build"
	@rm -rf $(OUT_DIR)/imach
	@echo "  cmach -> imach"
	@$(CMACH) build . -o out/linux/bin/imach
	@echo "intermediary compiler ready: $@"

# final compiler build (using imach)
# mach.toml determines target and output paths
$(MACH): $(IMACH)
	@echo "  cleaning artifacts for mach build"
	@rm -rf $(OUT_DIR)/mach
	@echo "  imach -> mach"
	@echo ""
	@echo "  NOTE: This stage of the pipeline is incomplete and included for scaffolding purposes"
	@echo ""
	@$(IMACH) build .
	@echo "final compiler ready: $@"
