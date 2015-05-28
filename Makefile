# build an executable named myprog from myprog.c
CC = gcc

CFLAGS = -g -Wall

TARGET = integralController

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)
