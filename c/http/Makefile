.SUFFIXES:
.SUFFIXES: .o .c
.PHONY: clean 


LDFLAGS = -g
LIBS = 
CFLAGS = -g -O2 -pipe -Wall -I.

CC = gcc
CPP = gcc -E

SOURCES = boa.c select.c request.c  queue.c buffer.c read.c

OBJS = $(SOURCES:.c=.o)

all: boa

boa:	$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) boa
