CC = gcc
CFLAGS = -Wall -O2

all: pipefifo

pipefifo: pipefifo.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f pipefifo