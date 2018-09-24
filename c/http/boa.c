#include "boa.h"
#include <sys/resource.h>
#include <sys/socket.h>
#include <netinet/tcp.h>


/* globals */
int backlog = SO_MAXCONN;
time_t start_time;


time_t current_time;
int max_fd = 0;
int pending_requests = 0;

static int sock_opt = 1;
static int do_fork = 1;
int devnullfd = 1;

int max_connection = 0;

int server_port = 8000;

long int max_connections = 100;
int main(int argc, char **argv)
{
	int c;
	int server_s;
	
	/* set umask to u+rw, u-x, go-rwx */
	c = umask(~0600);
	if(c == -1)
	{
		perror("umask");
		exit(1);
	}

	server_s = create_server_socket();
	

	if(max_connection < 1)
	{
		struct rlimit rl;
		
		/* 获取系统资源 */
		c = getrlimit(RLIMIT_NOFILE, &rl);
		if(c < 0)
		{
			perror("getrlimit");
			exit(1);
		}
		max_connection = rl.rlim_cur;

	}


	/* 创建子进程继续运行 */
	if(do_fork)
	{
		;
	}

	select_loop(server_s);	//select 非阻塞socket

	return 0;

}



int create_server_socket(void)
{
	int server_s;

	server_s = socket(SERVER_AF, SOCK_STREAM, IPPROTO_TCP);

	if(server_s == -1)
	{
		printf("socket err\r\n");
	}

	/* 设置非阻塞sock */
	if(set_nonblock_fd(server_s) == -1)
	{
		printf("set_nonblock err\r\n");
	}

	/* close server socket on exec so cgi's can't write to it*/
	if(fcntl(server_s, F_SETFD, 1) == -1)   //fcntl 修改描述符属性
	{
		printf("fcntl F_SETFD err\r\n");
	}

	/* reuse socket addr */
	if((setsockopt(server_s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
			sizeof(sock_opt))) == -1)
	{
		printf("setsockopt err\r\n");
	}

	
	if(bind_server(server_s, NULL) == -1)
	{
		printf("bind_server err\r\n");
	}
	
	if(listen(server_s, backlog) == -1)
	{
		printf("listen err\r\n");
	}
	
	printf("create socket ok\r\n");

	return server_s;		
}


int bind_server(int server_s, char *server_ip)
{
	
	struct sockaddr_in server_sockaddr;
	memset(&server_sockaddr, 0, sizeof(server_sockaddr));

	server_sockaddr.sin_family = AF_INET;

	if(server_ip != NULL)
	{
		inet_aton(server_ip, &server_sockaddr.sin_addr);
	}
	else
	{
		server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	}
	
	server_sockaddr.sin_port = htons(server_port);
	
	return bind(server_s, (struct sockaddr *) &server_sockaddr, 
					sizeof(server_sockaddr));
}

int net_port(struct SOCKADDR *s)
{
	int p = -1;
	p = ntohs(s->sin_port);
	return p;
}
