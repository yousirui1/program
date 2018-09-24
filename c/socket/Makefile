.all : clean

LDFLAGS := -L./lib/arm/*.so
LIBS := -lavformat -lswscale -lavcodec -lavutil
INCLUDE :=-I./include
#CC := arm-goke-linux-uclibcgnueabi-gcc
CC := gcc

socket: 
	$(CC) main.c cJSON.c parsecmd.c h264.c  -I ./ -lm  -lpthread -g  -o socket

#test.o: test.c
	#gcc test.c -g -o play -I ./include -L ./lib/x86   -lm -lpthread -lz 
	#$(CC) $(INCLUDE) -c $^
			
			
clean:
	rm -rf socket *.o
 
