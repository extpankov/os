CC = gcc
CFLAGS = -Wall -O2

all: pipe fifo_sender fifo_receiver
pipe: pipe.c
	$(CC) $(CFLAGS) -o $@ $<

fifo_sender: sender.c
	$(CC) $(CFLAGS) -o $@ $<

fifo_receiver: receiver.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f pipe fifo_sender fifo_receiver
