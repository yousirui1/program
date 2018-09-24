#include <stdio.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include "file.h"

#define SERVER_PORT 22222
#define SERVER_PATH "server.buf"

/*
 *	初始化建立socket
 */
int  init()
{
	struct sockaddr_in ser;
	int socket_fd, connect_fd;
	unsigned char ip_addr[16] = {0};
	unsigned char recv_buf[BUFFER_SIZE], send_buf[BUFFER_SIZE];

	if(file_init(SERVER_PATH))
	{
		printf("\r\n file err \r\n");
		return 1;
	}

	//初始化socket	
	socket_fd = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP
	if(socket_fd < 0)
	{
		printf("\r\n socket err \r\n");
		return -1;
	}
	
	#if 0
	int flag = 1;
	if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag))<0)
	{
		return -1;
	}	
	#endif
	//初始化结构体	
	ser.sin_family = AF_INET;
	ser.sin_port = htons(SERVER_PORT);
	//ser.sin_addr.s_addr = inet_addr();
	ser.sin_addr.s_addr =htons( INADDR_ANY);
	do{
		if(bind(socket_fd, (struct sockaddr *)&ser, sizeof(struct sockaddr))<0)
		{
			printf("\r\n bin err \r\n");
			break;
		}	

		if(listen(socket_fd, SOMAXCONN)<0)
		{
			printf("\r\n listen err \r\n");
			break;
		}	
	#if 0	
		if(get_ip(ip_addr)<0)
		{
			break;
		}	
	#endif
	}while(0);

	while(1)
	{
		if((connect_fd = accept(socket_fd, (struct sockaddr*)NULL,NULL) )== -1)
		{
			printf("\r\n accpte socket error\r\n");
			continue;
		}
		while(1)
		{

			if(!fork())
			{
			char send_buf[BUFFER_SIZE];
			scanf("%s", &send_buf);
			send(connect_fd,send_buf,BUFFER_SIZE, 0);
			//file_write(SERVER_PATH, "server:",send_buf);
			}

			memset(recv_buf, 0, sizeof(recv_buf));
			recv(connect_fd,recv_buf,BUFFER_SIZE,0); 
			//file_write(SERVER_PATH,"client:",recv_buf);
			if(NULL != recv_buf)
			{
				printf("client:%s\n",recv_buf);
			}

		}	
	}	
	
	close(connect_fd);	
	close(socket_fd);

}

int main()
{
	init();
	return 0;
}

























/*
 * 获取eth0的ip 
 */

int get_ip(unsigned char *ip_addr)
{
	struct ifaddrs *p_if_addr = NULL;
	struct ifaddrs *p_free_addr; 
	int status = -1;	
	
	if(NULL == ip_addr)
	{
		return -1;
	}
	
	if(getifaddrs(&p_if_addr) <0)
	{
		return -1;
	}
	
	p_free_addr = p_if_addr;

	while(NULL != p_if_addr)
	{
		printf("\r\n %d, %s,\r\n", p_if_addr->ifa_addr->sa_family,p_if_addr->ifa_name);
		if((AF_INET == p_if_addr->ifa_addr->sa_family) && (0 == strcmp(p_if_addr->ifa_name,"eth0")))
		{
			printf("\r\n sss%s \r\n", p_if_addr->ifa_addr);	
			//memcpy(ip_addr, , (sizeof(eth0_ip)+1));
			status = 0;
			break;
		}	
		
		if(NULL != p_free_addr)
		{
			freeifaddrs(p_free_addr);
		}
		if( -1 == status)
		{
			return -1;
		}
	}

	return status;	
}


