.all : clean

LDFLAGS := -L./lib/arm/*.so
LIBS := -lavformat -lswscale -lavcodec -lavutil
INCLUDE :=-I./include
#CC := arm-goke-linux-uclibcgnueabi-gcc
CC := gcc

socket: 
	$(CC) main.c    -I ./ -lm  -lpthread -g  -o server

#test.o: test.c
	#gcc test.c -g -o play -I ./include -L ./lib/x86   -lm -lpthread -lz 
	#$(CC) $(INCLUDE) -c $^
			
			
clean:
	rm -rf server *.o
 
