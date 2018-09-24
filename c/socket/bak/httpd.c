#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// --------------------------------------- 辅助变量宏 和 声明 ------------------------------------------

// char[]缓冲区大小
#define _INT_BUF (1024)
// listen监听队列的大小
#define _INT_LIS (7)


void response_200(int cfd);


void response_file(int cfd);

void response_app(int cfd);


int serstart();


void *request_accept(void *arg);


void request_cgi();


int main()
{
	pthread_attr_t attr;

	int sock_fd = serstat();

	//初始化线程属性
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	for(;;)
	{
		pthread_t th_id;
		struct sockaddr_in sock;
		
		socklen_t sock_size = sizeof(sock);

		//memset(&sock, 0 ,sizeof(sock));

		int fd = accept(sock_fd, (struct sockaddr*)&sock, &sock_size);
		if(fd < 0)
		{
			printf("accept is error!");
			break;
		}

		if(pthread_create(&th_id, &attr, request_accept, (void*) fd) <0)
		{
			printf("pthread_create run is error!");
		}	
	}
	
	pthread_attr_destroy(&attr);
	close(sock_fd);
	
	return 0;
}

void response_400(int fd)
{
	const char *str = "HTTP/1.0 400 BAD REQUEST\r\n"
				"Server: gk httpd\r\n"
				"Content-Type: text/html\r\n"
				"\r\n"
				"";
	write(fd, str, strlen(str));
}

void response_200(int fd)
{
	const char *str ="HTTP/1.0 200 OK\r\n"
				"Server: gk httpd\r\n"
				"Content-Type: text/html\r\n"
				"\r\n";

	char buf[1024] = {0};

	strcpy(buf, str);
	strcat(buf, "var name = \"ysr\";\r\nvar sex = \"man\";\r\n");
	write(fd, buf, strlen(buf));
	close(fd);
}

void response_file(int fd)
{	
	char buf = "var name = \"ysr\";\r\nvar sex = \"man\"; ";
	response_200(fd);	

	//write(fd, buf, strlen(buf));
	//send(fd, buf, strlen(buf), 0);
		
	
}

void response_app(int fd)
{	

	char *tmp1 = "test";
	char *tmp2 = "sent three times";

	const char *str ="HTTP/1.0 200 OK\r\n"
				"Server: gk httpd\r\n"
				"Content-Type: application/octet-stream\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 20\r\n"
				"\r\n";
	

	//char buf[1000] = {0};
	

	write(fd, str, strlen(str));
	write(fd, tmp1, strlen(tmp1));
	write(fd, tmp2, strlen(tmp2));
}
int serstat()
{
	int sock_fd;
	
	struct sockaddr_in sock_in;
	
	memset(&sock_in, 0, sizeof(sock_in));

	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket err\n");
	}

	sock_in.sin_port = htons(80);
	sock_in.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(sock_fd, (struct sockaddr*)&sock_in, sizeof(sock_in)) <0)
	{

		printf("bind err\n");
	}

	if(listen(sock_fd, _INT_LIS))
	{

		printf("listen err\n");
	}

	printf("sock ok %d", sock_fd);
	return sock_fd;	
		
}
int flag = 1;
void * request_accept(void *arg)
{
	int fd = (int)arg;
	//struct stat st;
	//char buf[_INT_BUF];

	if(flag == 0)
	{
		flag = 1;
		response_200(fd);
	}
	else
	{
		response_app(fd);
	}

	
	
}
