CFLAGS=-g -Wall
CC = gcc

TARGET = triangleWave
DEPENDS = triangleWave.o

all: $(TARGET)

$(TARGET): $(DEPENDS)
	$(CC) -o $(TARGET) $(DEPENDS) $(CFLAGS)

triangleWave.o: triangleWave.c
	$(CC) -c triangleWave.c $(CFLAGS)

clean: 
	rm -f $(TARGET) $(DEPENDS)
