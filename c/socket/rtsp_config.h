#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
 #include <arpa/inet.h>


#define SCHED_PRIORITY_DATAPROC 1

#define MAX_RTSP_CLIENT 1 

#define SERVER_SECTION "server"
#define SERVER_PORT "port"
#define MAX_RECV_LEN 2048 
#define SERVER_PORT_BASE 23612

#define MAX_ORDER_LEN 40
#define MAX_PORT_LEN 20
#define MAX_SESSION_LEN 24
#define MAX_ROWS 200
#define MAX_RESP_LEN 1024
#define MAX_TRANSPORT_LEN 128
#define MAX_SETUP_PARA_LEN 128
#define MAX_SDP_LEN 512


enum play_stat
{
	PLAY=1,
	PAUSE,
	STOP,	
};

struct frame_data
{
	char type; //视频 0，音频 1
	unsigned long long index;
	int size;
	char* data;
};

struct rtp_param
{
	unsigned short h264_seq_num;
	unsigned short pcm_seq_num;

	unsigned int h264_timestamp;
   	unsigned int pcm_timestamp;	
	
	struct timeval firstTime;
	struct timeval pcm_firstTime;
};

struct rtsp_para
{
	//具体指令：SETUP OPTIONS...
	char order[MAX_ORDER_LEN];
	int cseq;
	char session[MAX_SESSION_LEN];
	//SETUP指令所携带的transport信息
   	char transport[MAX_TRANSPORT_LEN];
	//每种指令所携带信息(如:SETUP rtsp://192.168.1.131:554/stream0/track0 RTSP/1.0\r\n)
	//则order_para = "rtsp://192.168.1.131:554/stream0/track0 RTSP/1.0"
	char order_para[MAX_SETUP_PARA_LEN];

	//SETUP指令所指定的port,双通道
	int client_port0;
	int client_port1;

	int serv_port0;
	int serv_port1;

	char istcp;
	int tcp_channel;	
};

struct serv_struct
{

	//客户端地址(TCP)
	struct sockaddr * client_sockaddr;

	//连接套接字
	int fd;

	struct rtsp_para * rtsp_pa; 
	int session_id;
	int udp_sock;
	int udp_sock1;
	struct sockaddr_in udp_sockaddr0;
	struct sockaddr_in udp_sockaddr1;


	enum play_stat playstat;	
	char server_ip[24];
	char client_ip[24];

	int save_opt1;
	int save_opt2;
	int save_opt3;
};
#endif

