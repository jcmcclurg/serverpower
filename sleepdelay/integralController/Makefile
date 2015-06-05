# build an executable named myprog from myprog.c
CC = gcc

CFLAGS = -g -Wall

TARGET = integralController
DEPENDS = commonFunctions.o

all: $(TARGET)

$(TARGET): $(TARGET).o $(DEPENDS)
	$(CC) -o $(TARGET) $(TARGET).o $(DEPENDS) $(CFLAGS) -lm

integralController.o: integralController.c
	$(CC) -c integralController.c $(CFLAGS)

commonFunctions.o: commonFunctions.c commonFunctions.h
	$(CC) -c commonFunctions.c $(CFLAGS)

clean:
	$(RM) $(TARGET) $(TARGET).o $(DEPENDS)
