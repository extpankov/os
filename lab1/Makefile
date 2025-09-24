CC = gcc
CFLAGS = -Wall -O2

all: mycat mygrep

mycat: mycat.c
	$(CC) $(CFLAGS) -o $@ $<

mygrep: mygrep.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f mycat mygrep *.o