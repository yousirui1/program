#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include "cJSON.h"
#include "rtsp_config.h"


#define BUFFER_SIZE 2048

time_t current_time;


struct rtp_param rtp_pa;

struct serv_struct *rtsp_client;


pthread_mutex_t client_mutex;

void login(int fd);
void select_loop(int fd);
void dec_header(int fd ,unsigned char *sendbuf, int len);
void LoginRet(unsigned char *buf, unsigned int len);
void SendRtspMsg( unsigned char *buf, unsigned int len);

int main()
{
	int server_s = 0;
	int i = 0, keepAlive = 1, nodelay = 1;
	server_s = create_server_socket();		

	
	rtp_pa.h264_seq_num = 0; 
    rtp_pa.pcm_seq_num = 0; 
    rtp_pa.h264_timestamp = 0; 
    rtp_pa.pcm_timestamp = 0; 
	gettimeofday(&rtp_pa.firstTime, NULL);

	rtsp_client = (struct serv_struct*)malloc(sizeof(struct serv_struct));

    rtsp_client->rtsp_pa = (struct rtsp_para*)malloc(sizeof(struct rtsp_para ));
    if(!(rtsp_client->rtsp_pa))
    {    
        free(rtsp_client);
        goto err; 
    }    
    rtsp_client->session_id = i; 
    rtsp_client->udp_sock = -1;
    rtsp_client->playstat = STOP;

    memset(rtsp_client->rtsp_pa,0,sizeof(struct rtsp_para));

    rtsp_client->rtsp_pa->client_port0 = -1;
    rtsp_client->rtsp_pa->client_port1 = -1;

    rtsp_client->rtsp_pa->serv_port0 = SERVER_PORT_BASE + i*8 ;//偶数端口
    rtsp_client->rtsp_pa->serv_port1 = rtsp_client->rtsp_pa->serv_port0 + 2 ;//偶数端口
    
    rtsp_client->rtsp_pa->istcp = 0 ;//是否通过TCP传输数据
    rtsp_client->rtsp_pa->tcp_channel = 0 ;//tcp通道
    rtsp_client->fd = server_s ;//tcp通道
    
    //strcpy(rtsp_client->client_ip,inet_ntoa(((struct sockaddr_in* )sock)->sin_addr));

    
    if( setsockopt(server_s, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive)) != 0) goto err; 
    //if( setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle)) != 0) goto err2;
    //if( setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval)) != 0) goto err2;
    //if( setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount)) != 0) goto err2;
    if( setsockopt(server_s, IPPROTO_TCP, TCP_NODELAY,(char*)&nodelay, sizeof(nodelay)) !=0) goto err;//禁用NAGLE算法

	login(server_s);
	select_loop(server_s);


   	printf("Client %s is connected\n",rtsp_client->client_ip);
    return 0;

err:
    if(rtsp_client)
    {
		free(rtsp_client);
    }
}

void login(int fd)
{

    unsigned char buf[2048] = {0};  

    unsigned char *ret = NULL;

    unsigned int buf_len = 0;


    cJSON *root = cJSON_CreateObject();    

    cJSON_AddItemToObject(root, "type", cJSON_CreateNumber(0));      //type = 0 设备
    cJSON_AddItemToObject(root, "deviceid", cJSON_CreateString("000999"));
    cJSON_AddItemToObject(root, "companyid", cJSON_CreateString("0002"));  //  

    ret = cJSON_Print(root);

    buf_len = strlen(ret);

    memcpy(&buf[8], ret, buf_len);

    cJSON_Delete(root);

    free(ret);

    buf[0] = 0xFF;
    *(unsigned short *)(&buf[1]) = 1;

    *(unsigned int *)(&buf[3]) = buf_len;
    buf[7] = 0x01;

	write(fd, buf, buf_len + 8);
	
}


int create_server_socket(void)
{
	int server_s = 0;
	struct sockaddr_in s_addr;

	char ip[20] = "127.0.0.1";
	int port = 22000;
	
	if((server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf("socket err\n");
		return 0;
	}

	bzero(&s_addr, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);


	if(inet_aton(ip, &s_addr.sin_addr) == 0)
	{
		printf("inet_aton err\n");
		return 0;
	}

	while(connect(server_s, (struct sockaddr_in *)&s_addr, sizeof(s_addr)) != 0)
	{
		printf("sleep 3\n");
		sleep(3);
	}

	printf("connect sucessful\n");

	return server_s;
}


void select_loop(int fd)
{
	fd_set fds;
	struct timeval tv;
	
	int retval, maxfd = -1;
	
	FD_ZERO(&fds);
	
	struct sockaddr_in remote_addr;
	int remote_addrlen = sizeof(struct sockaddr_in);

	char msg[2048] = {0};

	int len = 0;

	while(1)
	{
		tv.tv_sec = 1;
		tv.tv_usec = 0l;

		FD_SET(fd, &fds);

		if(select(maxfd + 1, &fds, NULL, NULL, &tv) == -1) 
        {   
            if(errno == EINTR)
                continue;
            else if(errno != EBADF)
                printf("select\n");
        }   
        time(&current_time);

		if(FD_ISSET(fd, &fds))
        {   
			memset(msg, 0, sizeof(msg));
			
			len = recv(fd, msg, 2048, 0);
			
			if(len > 0)
			{
				printf("recv: %s\n", &msg[20]);
				dec_header(fd, msg, len);
			}
         }
	}
}

void dec_header(int fd ,unsigned char *sendbuf, int len)
{
	      
    unsigned char tmp[5];
    unsigned int readPos = 0;
    unsigned short cmd = 0;
    unsigned int data_len = 0;
    unsigned char *ret = NULL;
   	
 	unsigned int client_port0 = -1; 
	unsigned int client_port1 = -1; 

    /* 校验位 1 0xFF */
    memset(tmp, 0, 5); 
    memcpy(tmp, sendbuf + readPos, 1); 
    readPos += 1;   

	printf("recv %s\n", &sendbuf[8]);


    /* 指令位  2 0x0001 */
    memset(tmp, 0, 5); 
    memcpy((short *)&cmd, sendbuf + readPos, 2); 
    readPos += 2;   


    /* 数据长度 4  */
    memset(tmp, 0, 5); 
    memcpy((long *)&data_len, sendbuf + readPos, 4); 
    readPos += 4;   

    /* 版本号 1 0x01 */
    memset(tmp, 0, 5); 
    memcpy(tmp, sendbuf + readPos, 1); 
    readPos += 1;   


	
    if(readPos + data_len != len)
    {   
		printf("packet len err\n");
        return ;
    }   

	switch(cmd)
	{
		//LoginRet
		case 2:
			LoginRet(sendbuf + readPos, data_len);	
			break;
		//SendRtspMsg
		case 14:
			SendRtspMsg(sendbuf + readPos, data_len);				
			break;	
		case 3:
			break;
		default:
			break;

	}	

}


unsigned int client_port0 = -1;
unsigned int client_port1 = -1;

void LoginRet(unsigned char *buf, unsigned int len)
{
	
	struct serv_struct *_serv;
	_serv = rtsp_client;
    cJSON *root = NULL;
    cJSON *item = NULL;
    
    root = cJSON_Parse(buf);

	item = cJSON_GetObjectItem(root, "ret");


	if(item->valueint)
	{	
		printf("connect err\n");
	}
       
    item = cJSON_GetObjectItem(root, "vidport");
	_serv->rtsp_pa->client_port0 = item->valueint;

    item = cJSON_GetObjectItem(root, "pcmport");
	_serv->rtsp_pa->client_port1 = item->valueint;

	printf("client0 %d client1 %d\n", client_port0, client_port1);

	cJSON_Delete(root);
}

void SendRtspMsg(unsigned char *buf, unsigned int len)
{
	printf("sendrtspMsg\n");
	struct serv_struct *_serv;
	_serv = rtsp_client;

	unsigned int readPos = 0;

	int opt = 0;

	memcpy((int *)&(_serv->save_opt1), buf+readPos, 4);
	readPos += 4;	

	memcpy((int *)&(_serv->save_opt2), buf+readPos, 4);
	readPos += 4;	

	memcpy((int *)&(_serv->save_opt3), buf+readPos, 4);
	readPos += 4;	

	printf("save_opt %d %d %d\n", _serv->save_opt1, _serv->save_opt2, _serv->save_opt3);
	len -= readPos;

	//解析收到的报文
	if(parse_cmd(rtsp_client, buf + readPos, len))
	{
		printf("parse_cmd err\n");
	}

	//组装并回复报文
	process_cmd(rtsp_client);
}


void get_free_num()
{
	//return 0;
}

