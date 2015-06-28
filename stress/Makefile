# build an executable named myprog from myprog.c
CC = gcc

CFLAGS = -lm -g -Wall

TARGET = cstress
#DEPENDS = 

all: $(TARGET)

$(TARGET): $(TARGET).o $(DEPENDS)
	$(CC) -o $(TARGET) $(TARGET).o $(DEPENDS) $(CFLAGS)

cstress.o: cstress.c
	$(CC) -c cstress.c $(CFLAGS)

cpustress.o: cpustress.c
	$(CC) -c cpustress.c $(CFLAGS)

cpustress2.o: cpustress2.c
	$(CC) -c cpustress2.c $(CFLAGS)

clean:
	$(RM) $(TARGET) $(TARGET).o $(DEPENDS)
