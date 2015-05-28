# build an executable named myprog from myprog.c
CC = gcc

CFLAGS = -g -Wall

TARGET = integralController
DEPENDS = commonFunctions.o

all: $(TARGET)

$(TARGET): $(TARGET).o $(DEPENDS)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o $(DEPENDS)

integralController.o: integralController.c
	gcc -c integralController.c

commonFunctions.o: commonFunctions.c commonFunctions.h
	gcc -c commonFunctions.c

clean:
	$(RM) $(TARGET) $(TARGET).o $(DEPENDS)
