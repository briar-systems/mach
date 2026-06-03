# mach build system
# builds:
#   1. acquire cmach            (pinned version auto-downloaded to out/bin)
#   2. intermediary compiler    (cmach builds src/ -> out/bin/imach)
#   3. self-hosted compiler     (imach builds src/ -> out/bin/smach)
#   4. final compiler           (smach builds src/ -> out/linux/bin/mach)

OUT := out
BIN := $(OUT)/bin

# cmach is the bootstrap seed: the exact pinned version is auto-downloaded so a
# stale system cmach can never silently break the bootstrap. override the seed
# explicitly with `make CMACH=/path/to/cmach` when you know it matches.
CMACH_VERSION ?= 0.10.2
CMACH         ?= $(BIN)/cmach
CMACH_URL      = https://github.com/octalide/mach-boot/releases/download/v$(CMACH_VERSION)/cmach

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
	@$(IMACH) build . -o $@ --artifacts smach/linux --emit-asm --emit-ir

$(MACH): $(SMACH)
	@rm -rf $(OUT)/linux
	@echo "  smach -> mach"
	@$(SMACH) build .

$(BIN):
	@mkdir -p $@

clean:
	@rm -rf $(OUT)
