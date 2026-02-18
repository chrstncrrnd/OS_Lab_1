# Executables
TARGETS = mycalc mydu

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -O2

# Default target
all: $(TARGETS)

# Generic rule: ejX <- ejX.c
%: %.c
	$(CC) $(CFLAGS) $< -o $@

# Clean
clean:
	rm -f $(TARGETS)
