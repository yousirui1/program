#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>


#define PORT 1500
#define BACKLOG 5	//最大监听数

int main()
{
	int sock_fd, new_fd;
	struct sockaddr_in my_addr;
	struct scokaddr_in their_addr;
	int sin_size;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);  //建立socket
	if(sock_fd == -1)
	{
		printf();
		return -1;
	}
	
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(PORT);
	my_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	bzero(&(my_addr.sin_zero), 8);
	
	if(bind(sock_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr))<0)
	{
		printf("");
		return -1;
	}
	
	listen(sock_fd, BACKLOG);
	
	while(1)
	{
		sin_size=sizeof(struct sockaddr_in);
		new_fd= accept(sock_fd, (struct sockaddr*)&their_addr, &sin_size);

		if(new_fd == -1)
		{
			printf();
		}
		else
		{
			printf();
			send(new_fd, "Hello World!", 12, 0);
		}

	}
}

