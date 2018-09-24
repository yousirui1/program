all: ser_sock


ser_sock: server.o file.o
	$(CC) $(LDFLAGS) $^  -g  -o $@
	rm -rf *.o
	

server.o: server.c 
file.o:file.c file.h



clean:
	rm -f *.o ser_sock

