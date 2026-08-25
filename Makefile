CC := gcc
CFLAGS := -D_DEFAULT_SOURCE -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -O2 -Iinclude
LDFLAGS := -lm
TARGET := can_generator
SOURCES := src/main.c src/dbc.c src/can_transport.c
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
