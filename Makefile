# mach build system
# builds:
#   1. acquire cmach            (auto-downloaded or user-provided)
#   2. intermediary compiler    (cmach builds src/ -> out/bin/imach)
#   3. self-hosted compiler     (imach builds src/ -> out/bin/smach)
#   4. final compiler           (smach builds src/ -> out/linux/bin/mach)

CMACH_VERSION ?= 0.9.10
CMACH         ?= $(shell command -v cmach 2>/dev/null)
CMACH_URL      = https://github.com/octalide/mach-boot/releases/download/v$(CMACH_VERSION)/cmach

OUT := out
BIN := $(OUT)/bin

# if CMACH is empty (not in PATH, not overridden), download it
ifeq ($(CMACH),)
  CMACH := $(BIN)/cmach
endif

IMACH := $(BIN)/imach
SMACH := $(BIN)/smach
MACH  := $(OUT)/linux/bin/mach

.PHONY: mach clean

mach: $(MACH)

# download cmach if it doesn't exist at the resolved path
$(BIN)/cmach:
	@mkdir -p $(BIN)
	@echo "downloading cmach v$(CMACH_VERSION)..."
	@curl -fsSL "$(CMACH_URL)" -o $@ && chmod +x $@

$(IMACH): $(CMACH) | $(BIN)
	@rm -rf $(OUT)/imach
	@echo "  cmach -> imach"
	@$(CMACH) build . -o $@ --artifacts imach/linux

$(SMACH): $(IMACH) | $(BIN)
	@rm -rf $(OUT)/smach
	@echo "  imach -> smach"
	@$(IMACH) build . -o $@ --artifacts smach/linux --emit-masm --emit-asm

$(MACH): $(SMACH)
	@rm -rf $(OUT)/linux
	@echo "  smach -> mach"
	@$(SMACH) build . --emit-masm --emit-asm

$(BIN):
	@mkdir -p $@

clean:
	@rm -rf $(OUT)
