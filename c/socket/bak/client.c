#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/shm.h>

#define BUFFER_SIZE 1024
#define CLIENTPORT 22222

/* 
 * 客户端conect 并循环读取数据
 */
int init()
{
	
	unsigned int uiRet = 0;
	struct sockaddr_in cli;
	int socket_fd = 0,connect_fd = 0; 
	char send_buf[BUFFER_SIZE],recv_buf[BUFFER_SIZE];
	char ip_Server[256];
	//gets(ip_Server);


	//open socket SOCK_DGRAM UDP  SOCK_STREAM TCP
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd< 0)
	{
		printf("\r\n socket err\r\n");
		return 1;
	} 
	//服务器端填充sockaddr结构
	cli.sin_family = AF_INET;
	cli.sin_port = htons(CLIENTPORT);	
	cli.sin_addr.s_addr =inet_addr("127.0.0.1"); //inet_addr("127.0.0.1");
	
	if(connect(socket_fd, (struct sockaddr *)&cli, sizeof(struct sockaddr))<0)
	{
		printf("\r\n connect err\r\n");
		return 1;
	}
#if 0	
	char tmp_buf[BUFFER_SIZE] = {0};

	while(1)
	{
		recv(connfd, tmp_buf, BUFFER_SIZE, MSG_WAITALL);
	}
#endif
	while(1)
	{	
		memset(recv_buf, 0, sizeof(recv_buf));
		recv(socket_fd, recv_buf, BUFFER_SIZE,0);
		if(NULL != recv_buf)
		{
			printf("server:%s \r\n", recv_buf);
		}
		if(!fork())
		{
			scanf("$s",&send_buf);
			if(send(socket_fd, send_buf ,BUFFER_SIZE,0) <0)
			{
				printf("\r\n send err \r\n");
			}
		}

	}
	
	close(socket_fd);
	close(connect_fd);
	return 0;	
}


int main()
{
 	init();
	return  0;	
	
}
