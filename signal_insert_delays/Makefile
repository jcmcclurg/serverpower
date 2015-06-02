CC?=gcc
CFLAGS?=-Wall -g -D_GNU_SOURCE
TARGETS=insertDelays
LIBS=commonFunctions.o get_children.o

UNAME := $(shell uname)

all::	$(TARGETS) $(LIBS)

insertDelays:	insertDelays.c $(LIBS)
	$(CC) -o insertDelays insertDelays.c $(LIBS) $(CFLAGS) -lrt

get_children.o: get_children.c get_children.h
	$(CC) -c get_children.c $(CFLAGS)

commonFunctions.o: commonFunctions.c commonFunctions.h
	$(CC) -c commonFunctions.c $(CFLAGS)

clean:
	rm -f *~ *.o $(TARGETS)

