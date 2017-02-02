TARGET = springer
CC = gcc
CFLAGS = -std=c99 -Wall

.PHONY: default all clean
default: $(TARGET)
all: default

chess.o: chess.c chess.h
	$(CC) chess.c $(CFLAGS) -c
	
springer: chess.o chess.h
	$(CC) main.c $(CFLAGS) chess.o -o springer
	
clean:
	-@rm chess.o 2>/dev/null
	