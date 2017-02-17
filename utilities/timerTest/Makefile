CFLAGS=-g -Wall -lrt
CC = gcc

TARGET = timerTest
DEPENDS = timerTest.o

all: $(TARGET)

$(TARGET): $(DEPENDS)
	$(CC) -o $(TARGET) $(DEPENDS) $(CFLAGS)

timerTest.o: timerTest.c
	$(CC) -c timerTest.c $(CFLAGS)

clean: 
	rm -f $(TARGET) $(DEPENDS)
