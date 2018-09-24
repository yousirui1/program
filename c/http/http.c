#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>


#define MAXLINE 80
#define SERVERPORT 8000
#define INET_ADDRSTRLEN 16


int main(int argc, char *argv[])
{
	int i, maxi, maxfd, listenfd, confd, sockfd;
	int nret, client[PD_SETSIZE];
	ssize_t n;
	fd_set rset, allset;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN};
	socklen_t clientaddr_len;
	struct sockaddr_in clientaddr, serveraddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0); //获得一个监听fd
	
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_adr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htonl(SERVERPORT);
	bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(listenfd, 128);
	
	maxfd = listenfd;
	maxi = -1;
	for(i = 0; i < FD_SETSIZE; i++)
			client[i] = -1;	

	FD_ZERO(&allset);	
	FD_SET(listenfd, &allset);  //
	
	for(; ;)
	{
		rset = allset;  //文件描述符集合
		nret = select(maxfd + 1, &rset, NULL, NULL);
		
		if(nret < 0)
		{
			if(FD_ISSET(listenfd, &rset))  //判断是否在文件描述符集合 
			{
				clientaddr_len = sizeof(clientaddr);
				confd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddr_len);
					
				
				for(i = 0; i < FD_SETSIZE; i++) //找空闲的client
				{
					if(client[i] < 0)
					{
						client[i] = confd;
						break;
					}

				}

				if(i == FD_SETSIZE)
				{
					exit(1);
				}

				FD_SET(confd, &allset);  //添加新的文件描述符
				if(confd > maxfd)
					maxfd = confd;
				if(i > maxi)
					maxi = i;
				if(--nret == 0)
					continue;	
			}

			for(i = 0; i<= maxi ; i++)
			{
				//检查那个clients 有数据就绪
				if((sockfd = client[i]) <0)  //遍历client 
					continue;
					
				if(FD_ISSET(sockfd, &rset)) //检测是否有数据
				{
					if((n = read(sockfd, buf, MAXLINE) == 0))
					{
						//当client 关闭连接时,服务端也关闭对应的连接
						close(fd);
						FD_CLR(sockfd, &allset);  //删除文件描述符
						client[i] = -1;			  //修改client 状态
					}
					else
					{
						//有数据到达的fd 
						int j;
						for( j = 0; j < n ; j++)
							

					}
					if(--nret == 0) //判断fd 是不最后一个
						break;
					
				}

			}

		}	
		close(listenfd);
		return 0;	
	}


}
