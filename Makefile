CC      = gcc
CFLAGS  = -Wall -Wextra -Wno-unused-parameter
LIBS    = -lraylib -lopengl32 -lgdi32 -lwinmm

SRCS    = gui.c main.c
TARGET  = gui.exe

$(TARGET): $(SRCS) sync.h
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)

console: main.c
	$(CC) main.c -o console.exe $(CFLAGS)

clean:
	rm -f $(TARGET) console.exe
