/*=============================================================================
#     FileName: tcpservselect.c
#         Desc: receive client data and then send they back.
#       Author: Licaibiao
#   LastChange: 2017-02-12 
=============================================================================*/
#include<stdio.h>  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<unistd.h>  
#include<stdlib.h>  
#include<errno.h>  
#include<arpa/inet.h>  
#include<netinet/in.h>  
#include<string.h>  
#include<signal.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#define MAXLINE	  1024
#define LISTENLEN 10
#define SERV_PORT 22000

void de_packet(int fd,  char *buf, int len);
 
int main(int argc, char **argv)
{
	int					i, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready, client[FD_SETSIZE];
	int 				sock_opt = 1;
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
 
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
 
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);
 
	if(setsockopt(listenfd, SOL_SOCKET, O_NONBLOCK, &sock_opt, sizeof( int ) ) == -1 )
    {
        printf( "\n vs_socket.c : setsockopt error! nonb socket_id=%d\n", listenfd );
        return -1;
    }


	bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));


	listen(listenfd, LISTENLEN);


 
	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
 
	for ( ; ; ) 
	{
		rset = allset;		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
 
		if (FD_ISSET(listenfd, &rset)) /* new client connection */
		{	
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
#ifdef	NOTDEF
			printf("new client: %s, port %d\n",
					inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
					ntohs(cliaddr.sin_port));
#endif
 
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE)
			{
				printf("too many clients");
				exit(0);
			}
 
			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */
 
			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}
 
		for (i = 0; i <= maxi; i++) 	/* check all clients for data */
		{	
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) 
			{
				if ( (n = read(sockfd, buf, MAXLINE)) == 0)/* connection closed by client */ 
				{
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} else
					//printf("msg :%s\n", &buf[8]);
					//printf("msg :%s\n", &buf[20]);
					de_packet(sockfd, buf, n);
 
				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}

int cli_fd;
int vlc_fd;
void de_packet(int fd,  char *buf, int len)
{
	int flag ;
	memcpy(&flag, buf, 1);	
	printf("flag %d\n", flag);
	if(flag == 255)   //device to vlc
	{
		printf(" device to vlc\n");
		cli_fd = fd;
		char tmp[5] = {0};
		short cmd = 0;
		int buf_len = 0;
		char ver;
		int save_opt1, save_opt2, save_opt3;
	
		int readPos = 0;
	
		memcpy(&flag, buf + readPos, 1);	
		readPos += 1;		

		memcpy(&cmd, buf +readPos, 2);
		readPos += 2;

		memcpy(&buf_len, buf +readPos, 4);
		readPos += 4;		

		memcpy(&ver, buf +readPos, 1);
		readPos += 1;		
	
		if(cmd == 2)
		{

			memcpy(&save_opt1, buf + readPos, 4);
			readPos += 4;		

			memcpy(&save_opt1, buf + readPos, 4);
			readPos += 4;		

			memcpy(&save_opt1, buf + readPos, 4);
			readPos += 4;		

			printf(" packet :%s\n", &buf[20]);
		}
		else
		{

			//printf(" packet :%s\n", &buf[8]);

		}
		
		write(vlc_fd, buf + readPos, len - readPos);
	}
	else			//vlc
	{
		vlc_fd = fd;

		printf(" vlc to deivce\n");
		printf("pack %s\n", buf);
				
		unsigned char msg[1024] = {0};
		msg[0] = 0xFF;
		*(unsigned short *)(&msg[1]) = 14;
	
		*(unsigned int *)(&msg[3]) = len + 12;
		msg[7] = 0x01;
		
		*(unsigned int *)(&msg[8]) = 1;

		*(unsigned int *)(&msg[12]) = 1;
	
		*(unsigned int *)(&msg[16]) = 1;

		
		memcpy(&msg[20], buf, len);
		write(cli_fd, msg, len  + 20);
	}
}
 











#if 0
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <fcntl.h>
#include <netinet/tcp.h>


#define BUFFER_SIZE 2048

time_t current_time;

int main()
{
	int server_s = 0;
	create_server_socket();
	return 0;

}


int create_server_socket(void)
{
	int vlc_s = 0, server_s = 0;
	struct sockaddr_in s_addr, vlc_addr;
	int sock_opt = 1;

 	if((server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {   
        printf("socket err\n");
        return 0;
    }   

    bzero(&s_addr, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(22000);

	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	
	if(bind(server_s, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1)
	{
		printf("server_s bind err\n");
		return 1;
	}


	if(listen(server_s, 5) == -1)
	{
		printf("server_s listen err\n");
		return 1;
	}


#if 0
 	if((vlc_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {   
        printf("socket err\n");
		return 1;
    }   

    bzero(&vlc_addr, sizeof(vlc_addr));
    vlc_addr.sin_family = AF_INET;
    vlc_addr.sin_port = htons(8888);

	vlc_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(vlc_s, (struct sockaddr *)&vlc_addr, sizeof(vlc_addr)) == -1)
	{
		printf("server_s bind err\n");
		return 1;
	}

	if(listen(vlc_s, 5) == -1)
	{
		printf("vlc_s listen err\n");
        return 1;
	}
#endif

#if 0
	if(fcntl(server_s, F_SETFL, O_NONBLOCK) == -1)
	{
		printf("fcntl err\n");
	}


 	if(setsockopt(server_s, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof( int ) ) == -1 )
    {
        printf( "\n vs_socket.c : setsockopt error! server_s\n");
        return -1;
    }
#endif
	
#if 0
	if(setsockopt(vlc_s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( int ) ) == -1 )
    {
        printf( "\n vs_socket.c : setsockopt error! socket_id=%d\n", vlc_s );
        return -1;
    }

	if(setsockopt(server_s, SOL_SOCKET, O_NONBLOCK, &opt, sizeof( int ) ) == -1 )
    {
        printf( "\n vs_socket.c : setsockopt error! nonb socket_id=%d\n", server_s );
        return -1;
    }

	if(setsockopt(vlc_s, SOL_SOCKET, O_NONBLOCK, &opt, sizeof( int ) ) == -1 )
    {
        printf( "\n vs_socket.c : setsockopt error! non socket_id=%d\n", vlc_s );
        return -1;
    }
#endif



	fd_set fds;
	struct timeval req_timeout;
	int max_fd = -1;
	
	FD_ZERO(&fds);

	struct sockaddr_in remoteaddr;
	unsigned char msg[1024] = {0};

	while(1)
	{
		max_fd = -1;
	
		FD_SET(server_s, &fds);
		//FD_SET(vlc_s, &fds);

		req_timeout.tv_sec = 1;
		req_timeout.tv_usec = 0;

		if(select(max_fd + 1, &fds, NULL, NULL, &req_timeout) == -1)
		{
			if(errno == EINTR)
				continue;
			else if(errno != EBADF)
				printf("select\n");
		}

		time(current_time);

		if(FD_ISSET(server_s, &fds))
		{
			
			int fd = accept(server_s, (struct sockaddr*)&remoteaddr, 
						sizeof(remoteaddr));
			printf("FD_ISSET server_s %d\n", fd);
			memset(msg, 0, sizeof(msg));

			int bytes = recv(fd, msg, 1024, 0);
	
			if(bytes > 0)
			{
				printf("recv: %s\n", msg);

			}
		}	


#if 0
		if(FD_ISSET(vlc_s, &fds))
		{
			//printf("FD_ISSET vlc_s\n");
		}
#endif
	}

	return 0;

}
#endif


