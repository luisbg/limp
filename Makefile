CC=gcc
CFLAGS=-I.

hellomake: limp.o
	$(CC) -o limp limp.o -I.
