CFLAGS=-g -Wall
CC = gcc

TARGET = playback
DEPENDS = playback.o

all: $(TARGET)

$(TARGET): $(DEPENDS)
	$(CC) -o $(TARGET) $(DEPENDS) $(CFLAGS) -lm -lrt

playback.o: playback.c
	$(CC) -c playback.c $(CFLAGS) -lm -lrt

clean: 
	rm -f $(TARGET) $(DEPENDS)
