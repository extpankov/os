CC = gcc
CFLAGS = -Wall -O2

all: sender receiver

sender: sender.c common.h
	$(CC) $(CFLAGS) -o $@ $<

receiver: receiver.c common.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f sender receiver
