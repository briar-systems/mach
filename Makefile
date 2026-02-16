# mach unified build system
# builds:
#   1. bootstrap c compiler    (boot/ -> out/bin/cmach)
#   2. intermediary compiler   (cmach builds src/ -> out/bin/imach)
#   3. self-hosted compiler    (imach builds src/ -> out/bin/smach)
#   4. final compiler          (smach builds src/ -> out/linux/bin/mach)
#
# directory structure:
#   out/
#     bin/              # stage binaries (cmach, imach, smach)
#     cmach/            # cmach artifacts (obj/)
#     imach/<target>/   # imach artifacts (ast/, ir/, asm/, obj/)
#     smach/<target>/   # smach artifacts (ast/, ir/, asm/, obj/)
#     mach/<target>/    # mach artifacts (ast/, ir/, asm/, obj/)

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

# executables
CMACH := $(BIN_DIR)/cmach
IMACH := $(BIN_DIR)/imach
SMACH := $(BIN_DIR)/smach
MACH := $(OUT_DIR)/linux/bin/mach

# bootstrap compiler sources
BOOT_SOURCES := $(shell find $(BOOT_SRC_DIR) -type f -name '*.c')
BOOT_OBJECTS := $(BOOT_SOURCES:$(BOOT_SRC_DIR)/%.c=$(CMACH_OBJ_DIR)/%.o)
BOOT_HEADERS := $(shell find $(BOOT_INC_DIR) -type f -name '*.h')

# main targets
.PHONY: help cmach-clean cmach-build cmach imach-clean imach-build imach smach-clean smach-build smach mach-clean mach-build mach full clean test

# default target: print help
.DEFAULT_GOAL := help

help:
	@echo "mach compiler build system"
	@echo ""
	@echo "stages:"
	@echo "  cmach        - bootstrap compiler (C)"
	@echo "  imach        - intermediary compiler (built by cmach)"
	@echo "  smach        - self-hosted compiler (built by imach)"
	@echo "  mach         - final compiler (built by smach, output per mach.toml)"
	@echo "  full         - build all stages"
	@echo ""
	@echo "each stage supports -clean and -build suffixes:"
	@echo "  <stage>-clean  - clean build artifacts"
	@echo "  <stage>-build  - build without cleaning"
	@echo "  <stage>        - clean and build"
	@echo ""
	@echo "meta:"
	@echo "  clean        - clean all build artifacts"
	@echo "  test         - run test suite"

# bootstrap compiler
cmach-clean:
	@echo "cleaning cmach"
	@rm -rf $(OUT_DIR)/cmach
	@rm -f $(CMACH)

cmach-build: $(CMACH)

cmach: cmach-clean cmach-build

test: cmach-build
	@./out/bin/cmach test

# intermediary compiler
imach-clean:
	@echo "cleaning imach"
	@rm -rf $(OUT_DIR)/imach
	@rm -f $(IMACH)

imach-build: $(IMACH)

imach: imach-clean imach-build

# self-hosted compiler
smach-clean:
	@echo "cleaning smach"
	@rm -rf $(OUT_DIR)/smach
	@rm -f $(SMACH)

smach-build: $(SMACH)

smach: smach-clean smach-build

# final compiler
mach-clean:
	@echo "cleaning mach"
	@rm -rf $(OUT_DIR)/mach
	@rm -f $(MACH)

mach-build: $(MACH)

mach: mach-clean mach-build

# full build: all 4 stages
full: cmach imach smach mach

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
$(IMACH): $(CMACH) | $(BIN_DIR)
	@echo "  cleaning artifacts for imach build"
	@rm -rf $(OUT_DIR)/imach
	@echo "  cmach -> imach"
	@$(CMACH) build . -o $(IMACH)
	@echo "intermediary compiler ready: $@"

# self-hosted compiler build (using imach)
$(SMACH): $(IMACH) | $(BIN_DIR)
	@echo "  cleaning artifacts for smach build"
	@rm -rf $(OUT_DIR)/smach
	@echo "  imach -> smach"
	@$(IMACH) build . -o $(SMACH)
	@echo "self-hosted compiler ready: $@"

# final compiler build (using smach)
# smach is a different binary from the output, so no ETXTBSY
$(MACH): $(SMACH)
	@echo "  cleaning artifacts for mach build"
	@rm -rf $(OUT_DIR)/mach
	@echo "  smach -> mach"
	@$(SMACH) build .
	@echo "final compiler ready: $@"
