#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>




#define BUFFER_SIZE 1024
int main()
{
	int fd[2];
	int status = 0;
	char temp_buf[BUFFER_SIZE];
	char buf[BUFFER_SIZE];
	pipe(fd);
	
	while((status = fork()) == -1);
	
	if(0 == status)
	{
		sprintf(temp_buf,"This is an example of pipe!/n");	
		write(fd[1],temp_buf,BUFFER_SIZE);
	}
	else
	{
		wait(0);
		read(fd[0],buf,BUFFER_SIZE);
		printf("\r\n %s \r\n",buf);
	}
	
	return 0;
}
