.PHONY: all clean cf val
.SILENT: all clean

# Compilation objects
SRC = scan config       # all names (without extensions) of C files
SRC_O = $(SRC:%=%.o)

# Compilation settings
CC = gcc
CFLAGS = -D_DEFAULT_SOURCE -Wall -Wextra -Werror -std=c11 -O2
CPOSTFLAGS = -lm -lpthread   # these flags must be placed at the end

# Default target
all: clean scan.run

# Style check (not formatting, just checking)
cf:
	cp ../materials/linters/.clang-format .
	clang-format -n *.c *.h

# Memory leak check
val:
	valgrind --leak-check=full   # what are we checking?

# Linking
%.run: $(SRC_O)
	@$(CC) $(CFLAGS) $^ -o $@ $(CPOSTFLAGS)

# Compilation (without linking)
%.o: %.c %.h
	@$(CC) $(CFLAGS) -c $< -o $@ $(CPOSTFLAGS)

# Cleanup
clean:
	@rm -f *.o *.run

# For convenience, to avoid typos
clear: clean
