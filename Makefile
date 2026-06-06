# Compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -Wno-unused-parameter
LIBS    = -lraylib -lopengl32 -lgdi32 -lwinmm

# Source files and output
SRCS    = gui.c main.c
TARGET  = gui.exe

# Default target — built when you just type 'make'
$(TARGET): $(SRCS) sync.h
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)

# Console-only build (no GUI, original main.c behaviour)
console: main.c
	$(CC) main.c -o console.exe $(CFLAGS)

# Remove compiled output
clean:
	rm -f $(TARGET) console.exe
