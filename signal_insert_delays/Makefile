CC?=gcc
CFLAGS?=-Wall -g -D_GNU_SOURCE
TARGETS=insertDelays
LIBS=get_children.o trie.o

UNAME := $(shell uname)

all::	$(TARGETS) $(LIBS)

insertDelays:	insertDelays.c $(LIBS)
	$(CC) -o insertDelays insertDelays.c $(LIBS) $(CFLAGS) $(FLAGS)

get_children.o: get_children.c get_children.h
	$(CC) -c get_children.c $(CFLAGS) $(FLAGS)

trie.o: trie.c trie.h
	$(CC) -c trie.c $(CFLAGS) $(FLAGS)

#commonFunctions.o: commonFunctions.c commonFunctions.h
#	$(CC) -c commonFunctions.c $(CFLAGS) $(FLAGS)

triangleWave:	triangleWave.c
	$(CC) -o triangleWave triangleWave.c $(CFLAGS) $(FLAGS)

#sig_catcher: sig_catcher.c
#	$(CC) -o sig_catcher sig_catcher.c $(CFLAGS) $(FLAGS)

clean:
	rm -f *~ *.o $(TARGETS) triangleWave
