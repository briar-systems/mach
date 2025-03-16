CC = clang

CFLAGS = -Wall -Wextra -Wno-unused -std=c23

ifdef DEBUG
  CFLAGS += -g -O0
else
  CFLAGS += -O2
endif

SRCDIR = src
BUILDDIR = out
BINDIR = .
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

EXEC = $(BINDIR)/cmach
ifeq ($(OS),Windows_NT)
    EXEC = $(BINDIR)/cmach.exe
endif

all: $(BUILDDIR) $(EXEC)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)

.PHONY: all clean
