#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>



#define DEST_PORT 1500 // 目标地址端口号
#define DEST_IP "127.0.0.1"
#define MAX_DATA 100  // 接受到的数据最大程度

int main()
{
	int sock_fd, new_fd;

	struct sockaddr_in, dest_addr;
	char buf[MAX_DATA];
	
	sock_fd= socket(AF_INET, SOCK_STREAM, 0); //建立socket
	
	if(sock_fd == -1)
	{

	}

	dest_addr.sin_family=AF_INET;
	dest_addr.sin_port = htons(DEST_PORT);	
	dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);
	bzero(&(dest_addr.sin_zero),8);


	if(connect(sock_fd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr))==1)
	{
		printf("connect failed:%d",erron);
	}

	
	else
	{
		printf("connect success");
		recv(sock_fd, buf, MAX_DAT, 0);
		printf("%s", buf);
	}

	close(sockfd);
	return 0;

}
