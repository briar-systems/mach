# mach unified build system
# builds:
#   1. bootstrap c compiler (boot/ -> out/boot/bin/cmach)
#   2. intermediary compiler with cmach (src/ -> out/bin/imach)
#   3. final compiler with imach (src/ -> out/bin/mach)

# compiler and flags
CC := clang
CFLAGS := -std=c23 -Wall -Wextra -Werror -pedantic -O2
CFLAGS_DEBUG := -std=c23 -Wall -Wextra -Werror -pedantic -g -O0 -DDEBUG

# llvm configuration
LLVM_CFLAGS := $(shell llvm-config --cflags)
LLVM_LDFLAGS := $(shell llvm-config --ldflags --libs core)

OUT_DIR := out

# standard library (source only, referenced directly)
STD_DIR := std
MACH_SRC_DIR := src

# bootstrap compiler
BOOT_DIR := boot
CMACH_OUT_DIR := $(OUT_DIR)/cmach
CMACH_OBJ_DIR := $(CMACH_OUT_DIR)/obj
CMACH_BIN_DIR := $(CMACH_OUT_DIR)/bin
CMACH := $(CMACH_BIN_DIR)/cmach

# intermediary compiler (built with cmach)
IMACH_OUT_DIR := $(OUT_DIR)/imach
IMACH_OBJ_DIR := $(IMACH_OUT_DIR)/obj
IMACH_BIN_DIR := $(IMACH_OUT_DIR)/bin
IMACH_EXE := $(IMACH_BIN_DIR)/imach

# final compiler (built with imach)
MACH_OUT_DIR := $(OUT_DIR)/mach
MACH_OBJ_DIR := $(MACH_OUT_DIR)/obj
MACH_BIN_DIR := $(MACH_OUT_DIR)/bin
MACH_EXE := $(MACH_BIN_DIR)/mach

# compiler flags for each build stage
CMACH_FLAGS := -I $(MACH_SRC_DIR) -M mach=$(MACH_SRC_DIR) -I $(STD_DIR) -M std=$(STD_DIR) --obj-dir=$(IMACH_OBJ_DIR) --dep-dir=$(IMACH_OBJ_DIR) --emit-asm --emit-ast --emit-ir
MACH_FLAGS := 

# bootstrap compiler sources
BOOT_SOURCES := $(wildcard $(BOOT_DIR)/*.c)
BOOT_OBJECTS := $(BOOT_SOURCES:$(BOOT_DIR)/%.c=$(CMACH_OBJ_DIR)/%.o)
BOOT_HEADERS := $(wildcard $(BOOT_DIR)/*.h)

# entry point for mach compiler source
MACH_MAIN := $(MACH_SRC_DIR)/main.mach

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

# bootstrap compiler
cmach-clean:
	@echo "cleaning cmach"
	@rm -rf $(CMACH_OUT_DIR)

cmach-build: $(CMACH)

cmach: cmach-clean cmach-build

# intermediary compiler
imach-clean:
	@echo "cleaning imach"
	@rm -rf $(IMACH_OUT_DIR)

imach-build: $(IMACH_EXE)

imach: imach-clean imach-build

# final compiler
mach-clean:
	@echo "cleaning mach"
	@rm -rf $(MACH_OUT_DIR)

mach-build: $(MACH_EXE)

mach: mach-clean mach-build

# clean everything
clean:
	@echo "cleaning all"
	@rm -rf $(OUT_DIR)

# bootstrap compiler build
$(CMACH_BIN_DIR):
	@mkdir -p $(CMACH_BIN_DIR)

$(CMACH_OBJ_DIR):
	@mkdir -p $(CMACH_OBJ_DIR)

$(CMACH_OBJ_DIR)/%.o: $(BOOT_DIR)/%.c $(BOOT_HEADERS) | $(CMACH_OBJ_DIR)
	@echo "  cc  $<"
	@$(CC) $(CFLAGS) $(LLVM_CFLAGS) -I$(BOOT_DIR) -c $< -o $@

$(CMACH): $(BOOT_OBJECTS) | $(CMACH_BIN_DIR)
	@echo "  ld  $@"
	@$(CC) $(BOOT_OBJECTS) $(LLVM_LDFLAGS) -o $@
	@echo "bootstrap compiler ready: $@"

# intermediary compiler build (using cmach)
$(IMACH_BIN_DIR):
	@mkdir -p $(IMACH_BIN_DIR)

$(IMACH_OBJ_DIR):
	@mkdir -p $(IMACH_OBJ_DIR)

$(IMACH_EXE): $(MACH_MAIN) $(CMACH) | $(IMACH_BIN_DIR) $(IMACH_OBJ_DIR)
	@echo "  cmach -> imach"
	@$(CMACH) build $(MACH_MAIN) $(CMACH_FLAGS) -o $@
	@echo "intermediary compiler ready: $@"

# final compiler build (using imach)
$(MACH_BIN_DIR):
	@mkdir -p $(MACH_BIN_DIR)

$(MACH_EXE): $(MACH_MAIN) $(IMACH_EXE) | $(MACH_BIN_DIR)
	@echo "  imach -> mach"
	@$(IMACH_EXE) build $(MACH_MAIN) $(MACH_FLAGS) -o $@
	@echo "final compiler ready: $@"
