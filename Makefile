# Compiler and tools
CC       := gcc
RM       := rm -f

# Compiler flags
CPPFLAGS := -D_DEFAULT_SOURCE -Iinclude
CFLAGS   := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -O2
LDFLAGS  :=
LDLIBS   := -lm

# Project files
TARGET  := can_generator
SOURCES := src/main.c \
           src/dbc.c 
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean rebuild

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Compile each source file separately
src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Remove generated files
clean:
	$(RM) $(OBJECTS) $(TARGET)

# Clean and build from scratch
rebuild: clean all
