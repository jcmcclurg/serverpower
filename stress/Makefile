# build an executable named myprog from myprog.c
CC = gcc

CFLAGS = -lrt -lm -g -Wall

TARGET = cstress
#DEPENDS = 

all: $(TARGET)

$(TARGET): $(TARGET).o $(DEPENDS)
	$(CC) -o $(TARGET) $(TARGET).o $(DEPENDS) $(CFLAGS)

cstress.o: cstress.c
	$(CC) -c cstress.c $(CFLAGS)

clean:
	$(RM) $(TARGET) $(TARGET).o $(DEPENDS)
