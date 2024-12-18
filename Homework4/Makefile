CC = gcc
CCOPTS = -c -g -Wall -ggdb
LINKOPTS = -g -pthread

all: enzyme

enzyme: enzyme.o 
	$(CC) $(LINKOPTS) -o $@ $^

enzyme.o: enzyme.c enzyme.h
	$(CC) $(CCOPTS) -o $@ enzyme.c

clean:
	rm -rf enzyme enzyme.exe *.o *~ *.err *.out
