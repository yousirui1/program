#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "rtsp_config.h"
#include "parsecmd.h"
#include "typedef.h"
#include "h264.h"

void SendErrmsg(struct serv_struct * _serv);
void sprintf_date(char* ptr);

extern struct rtp_param rtp_pa;
extern struct serv_struct *rtsp_client;

//命令解析
//返回值，0 正常，1 错误
int parse_cmd(struct serv_struct * _serv,char* msg,int len)
{
	int i,j, ret = 0;

	char para[MAX_ORDER_LEN] = {0};
	char buf[MAX_TRANSPORT_LEN] = {0};
	int index = 0;
	char* rows[MAX_ROWS] = {0};

	j = 0;
	int rownum = 0;
	for(i = 0 ;i< len; i++)
	{
		if(msg[i] == 0x24)
			break;
		//以"\r\n"为分割
		if(msg[i] == '\r')
		{
			if( (i+1) < len && msg[i+1] == '\n' )
			{
				msg[i] = '\0';
				msg[i+1] = '\0';
				i++;
				rownum++;
			}
			else
			{
				return 1;
			}
		}
	}
	if(rownum > MAX_ROWS)
	{
		return 1;
	}
	char* ptr = msg;
	for(i = 0; i<rownum; i++)
	{
		rows[i] = ptr;
		ptr += strlen(rows[i]) +2;	
	}
	for(index = 0; index<rownum;index++)
	{
		if(0 == strcmp(rows[index],""))
		{
			index++;
			continue;
		}

		j = 0;
		memset(para,0,sizeof(para));

		for(i = 0; i<strlen(rows[index]); i++)
		{
			//第一行，order	
			if(index == 0)
			{
				if(rows[index][i] == ' ')
					break;
			}
			else
			{
				//其它行，各个参数
				if(rows[index][i] == ':')
					break;
			}
			if((j+1) >= MAX_ORDER_LEN)
			{
				return 1;			
			}
			para[j] = rows[index][i];
			j++;
		}

		//读出剩下的信息
		j = 0;
		memset(buf,0,sizeof(buf));
		while(i<strlen(rows[index]) )
		{
			if(index != 0)
			{
				if(rows[index][i] != ':' && rows[index][i] != ' ' && rows[index][i] != '	')
				{
					if(j >= sizeof(buf) )
					{
						return 1;
					}
					buf[j] = rows[index][i];
					j++;
				}
			}
			else
			{
				if(j == 0 && rows[index][i] == ' ')
				{
					i++;
					continue;
				}
				if(j >= sizeof(buf) )
				{
					return 1;
				}
				if(rows[index][i] == ' ')
					break;

				buf[j] = rows[index][i];
				j++;		
			}
			i++;
		}

		if(index == 0)
		{
			//第一行时具体的指令：OPTIONS SETUP PLAY ...
			strcpy_order(_serv,para);
			strcpy_order_para(_serv,buf);
		}
		else
		{
			//读出seq
			if(0 == strcmp(para,PARA_CSEQ))
				sscanf_cseq(buf,_serv);
			else if(0 == strcmp(para,PARA_SESSION))
				strcpy_session(_serv,buf);
			else if(0 == strcmp(para,PARA_TRANSPORT))
				strcpy_transport(_serv,buf);
		}
	}

	return ret;
}

void process_cmd(struct serv_struct * _serv)
{
	char *tokenPtr = NULL;
	char * outer_ptr = NULL;
	char para[MAX_ORDER_LEN] = {0};
	char tmp_transport[MAX_TRANSPORT_LEN] = {0};		
	char sdp[MAX_SDP_LEN] = {0};

	char client_port[MAX_PORT_LEN] = {0};

	int which_track = 0;
	int j, i = 0;
	char* tmp;

	char buf[MAX_RESP_LEN] = {0};
	char* ptr = buf;
	int sl = 0;

	sprintf(ptr,"%s", "RTSP/1.0 200 OK");
	sl = strlen(ptr);
	ptr[sl] = '\r';
	ptr[sl+1] = '\n';
	ptr += sl + 2;

	sprintf_cseq(ptr,_serv);
	sl = strlen(ptr);
	ptr[sl] = '\r';
	ptr[sl+1] = '\n';
	ptr += sl + 2;

	if(0 == strcmp_options(_serv))
	{
		sprintf(ptr,"%s", "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;
		ptr[0] = '\r';
		ptr[1] = '\n';
		ptr += 2;

		DEBUG("SEND OPTIONS ----> %d bytes resp : %s \n",ptr-buf,buf);
		client_bev_write(_serv, buf,ptr-buf);
	}
	else if(0 == strcmp_describe(_serv))
	{
		//写入时间
		sprintf_date(ptr);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;

		sprintf(ptr,"%s","Content-type: application/sdp");

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;

		//构造sdp协议信息
		tmp = sdp;
		sprintf(tmp,"%s","v=0");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		char cip[24] = {0};
		get_client_ip(_serv,cip);
		sprintf(tmp,"o=- %d %d IN IP4 %s",get_session_id(_serv)+1,1,cip);

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"s=%s","streamed by the Santachi RTSP server");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"t=%s","0 0");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"m=%s","video 0 RTP/AVP 96");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"c=%s","IN IP4 0.0.0.0");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"a=%s","rtpmap:96 H264/90000");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"a=%s","control:track0");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"a=%s","fmtp:96 packetization-mode=1;profile-level-id=4D0029;sprop-parameter-sets=R00AKZmwHgCJ+WEAAAMD6AAAdTCE,SOpDyA==");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"m=%s","audio 0 RTP/AVP 8");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"a=%s","rtpmap:8 PCMA/16000/1");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"a=%s","control:track1");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(tmp,"a=%s","range:npt=0-");

		sl = strlen(tmp);
		tmp[sl] = '\r';
		tmp[sl+1] = '\n';
		tmp += sl+2;

		sprintf(ptr,"Content-length: %d",tmp - sdp );

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;

		ptr[0] = '\r';
		ptr[1] = '\n';
		ptr += 2;

		memcpy(ptr,sdp,tmp-sdp);

		DEBUG("SEND  ---->DESCRIBE %d bytes resp : %s \n",ptr-buf+tmp-sdp,buf);
		client_bev_write(_serv, buf,ptr-buf+tmp-sdp);
	}
	else if(0 == strcmp_setup(_serv))
	{
		//有俩通道，track1 视频，track2 音频
		//通过查看order_para,是建立哪个通道的连接
		which_track = get_which_track(_serv);
		//是否tcp
		memset(tmp_transport,0,MAX_TRANSPORT_LEN);
		memcpy_transport(tmp_transport,_serv);

		tokenPtr=strtok_r(tmp_transport,";",&outer_ptr);	

		while(tokenPtr != NULL)
		{
			j = 0;
			memset(para,0,sizeof(para));
			for(i=0;i<strlen(tokenPtr); i++)
			{
				if(tokenPtr[i] == '=')
					break;

				if( j >= MAX_ORDER_LEN)
					break;

				para[j] = tokenPtr[i];
				j++;
			}		
			if(0 == strcmp(para,"interleaved"))
			{
				j = 0;
				memset(client_port,0,sizeof(client_port));
				for(;i<strlen(tokenPtr); i++)
				{
					if(tokenPtr[i] == '=')
						continue;

					if( j >= MAX_ORDER_LEN)
						break;

					client_port[j] = tokenPtr[i];
					j++	;
				}
				break;
			}
			tokenPtr = strtok_r(NULL,";",&outer_ptr); 
		}
		if(strcmp(client_port,"") && sscanf_tcp_channel(client_port,_serv) >=0)
			set_client_istcp(_serv);

		//写入时间
		sprintf_date(ptr);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl+2 ;

		sprintf_session_id(ptr,_serv);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;

		if(get_client_istcp(_serv) == 0)
			sprintf(ptr,"%s","Transport: RTP/AVP;unicast;");
		else
			sprintf(ptr,"%s","Transport: RTP/AVP/TCP;unicast;");

		sl = strlen(ptr);
		ptr += sl ;

		if(get_client_istcp(_serv) == 0)
		{
			//UDP传输，获取udp端口
			memset(tmp_transport,0,MAX_TRANSPORT_LEN);
			memcpy_transport(tmp_transport,_serv);

			tokenPtr=strtok_r(tmp_transport,";",&outer_ptr);	

			while(tokenPtr != NULL)
			{
				j = 0;
				memset(para,0,sizeof(para));
				for(i=0;i<strlen(tokenPtr); i++)
				{
					if(tokenPtr[i] == '=')
						break;

					if( j >= MAX_ORDER_LEN)
					{
						SendErrmsg(_serv);
						return;
					}
					para[j] = tokenPtr[i];
					j++;
				}		
				if(0 == strcmp(para,"client_port"))
				{
					j = 0;
					memset(client_port,0,sizeof(client_port));
					for(;i<strlen(tokenPtr); i++)
					{
						if(tokenPtr[i] == '=')
							continue;

						if( j >= MAX_ORDER_LEN)
						{
							SendErrmsg(_serv);
							return;
						}
						client_port[j] = tokenPtr[i];
						j++	;
					}
					break;
				}
				tokenPtr = strtok_r(NULL,";",&outer_ptr); 
			}
			if(sscanf_client_port(client_port,_serv,which_track) <=0 )
			{
				SendErrmsg(_serv);
				return;
			}
			if(0 == which_track)
				//	sprintf(ptr,"client_port=%s;server_port=%d-%d",client_port,_serv->rtsp_pa->serv_port0,_serv->rtsp_pa->serv_port0+1);
				sprintf(ptr,"client_port=%s",client_port);
			else if(1 == which_track)
				//	sprintf(ptr,"client_port=%s;server_port=%d-%d",client_port,_serv->rtsp_pa->serv_port1,_serv->rtsp_pa->serv_port1+1);
				sprintf(ptr,"client_port=%s",client_port);
		}
		if(get_client_istcp(_serv) == 1)
			sprintf(ptr,"interleaved=%s",client_port);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;

		ptr[0] = '\r';
		ptr[1] = '\n';
		ptr += 2;

		//setup过程回应之前，必须先做好准备，客户端收到回应后会立刻发送Play命令
		DEBUG("SEND ----> SETUP %d bytes resp : %s \n",ptr-buf,buf);
		client_bev_write(_serv, buf,ptr-buf);
	}
	else if(0 == strcmp_play(_serv))
	{
		//写入时间
		sprintf_date(ptr);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl+2 ;

		sprintf(ptr,"%s","Range: npt=0.000-");

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl+2 ;

		sprintf_session_id(ptr,_serv);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;

		ptr[0] = '\r';
		ptr[1] = '\n';
		ptr += 2;

		DEBUG("SEND ----> PLAY %d bytes resp : %s \n",ptr-buf,buf);
		client_bev_write(_serv, buf,ptr-buf);

		//设置可以播放的标识
		set_client_stat(_serv,PLAY);
	}
	else if( 0 == strcmp_get_paramter(_serv))
	{
		//写入时间
		sprintf_date(ptr);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl+2 ;

		sprintf_session_id(ptr,_serv);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;

		ptr[0] = '\r';
		ptr[1] = '\n';
		ptr += 2;

		DEBUG("SEND ---->GET_PARAMETER %d bytes resp : %s \n",ptr-buf,buf);
		client_bev_write(_serv, buf,ptr-buf);
	}
	else if( 0 == strcmp_teardown(_serv))
	{
		DEBUG("recv teardown,reset state to STOP\n");
		set_client_stat(_serv,STOP);
	}
	else if( 0 == strcmp_pause(_serv))
	{
		set_client_stat(_serv,STOP);
		//写入时间
		sprintf_date(ptr);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl+2 ;

		sprintf_session_id(ptr,_serv);

		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl+1] = '\n';
		ptr += sl + 2;

		ptr[0] = '\r';
		ptr[1] = '\n';
		ptr += 2;

		DEBUG("SEND ---->PAUSE %d bytes resp : %s \n",ptr-buf,buf);
		client_bev_write(_serv, buf,ptr-buf);
	}
}

void SendErrmsg(struct serv_struct * _serv)
{
	char * err = SERV_ERROR_RESP;

	char buf[64] = {0};
	sprintf(buf,"%s - %s",_serv->rtsp_pa->order,err);
	DEBUG("%s\n",buf);
	client_bev_write(_serv, err,strlen(err));
}

void sprintf_date(char* ptr)
{
	time_t rawtime;
	struct tm * timeinfo;
	char szTemp[50]={0};

	time(&rawtime); 
	timeinfo = gmtime(&rawtime);		
	strftime(szTemp,sizeof(szTemp),"%a, %d %b %Y %H:%M:%S GMT",timeinfo);
	sprintf(ptr,"Date: %s", szTemp);
}

void strcpy_order(struct serv_struct* _serv,char* para)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
	{
		memset(_serv->rtsp_pa->order,0,sizeof(_serv->rtsp_pa->order));
		strcpy(_serv->rtsp_pa->order,para);
	}
	pthread_mutex_unlock(&client_mutex);
}

void strcpy_order_para(struct serv_struct* _serv,char* para)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
	{
		memset(_serv->rtsp_pa->order_para,0,sizeof(_serv->rtsp_pa->order_para));
		strcpy(_serv->rtsp_pa->order_para,para);
	}
	pthread_mutex_unlock(&client_mutex);
}

void sscanf_cseq(char* buf,struct serv_struct* _serv)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		sscanf(buf,"%d",&_serv->rtsp_pa->cseq);
	pthread_mutex_unlock(&client_mutex);
}

void strcpy_session(struct serv_struct* _serv,char* para)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
	{
		memset(_serv->rtsp_pa->session,0,sizeof(_serv->rtsp_pa->session));
		strcpy(_serv->rtsp_pa->session,para);
	}
	pthread_mutex_unlock(&client_mutex);
}
void strcpy_transport(struct serv_struct* _serv,char* para)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
	{
		memset(_serv->rtsp_pa->transport,0,sizeof(_serv->rtsp_pa->transport));
		strcpy(_serv->rtsp_pa->transport,para);
	}
	pthread_mutex_unlock(&client_mutex);
}

void sprintf_cseq(char* ptr,struct serv_struct* _serv)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		sprintf(ptr,"CSeq: %d",_serv->rtsp_pa->cseq);
	pthread_mutex_unlock(&client_mutex);
}

int strcmp_options(struct serv_struct* _serv)
{
	int ret = 1;
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		ret = strcmp(_serv->rtsp_pa->order,ORD_OPTIONS);
	pthread_mutex_unlock(&client_mutex);

	return ret;
}

void client_bev_write(struct serv_struct* _serv, char* buf,int len)
{
	char data_buf[1024] = {0};
	data_buf[0] = 0xFF;	
	*(unsigned short *)(&buf[1]) = 2;
	*(unsigned int *)(&buf[3]) = len;

	data_buf[7] = 0x01;

	//*(unsigned int *)(&buf[8]) = _serv->save_opt1;
	//*(unsigned int *)(&buf[12]) = _serv->save_opt2;
	//*(unsigned int *)(&buf[16]) = _serv->save_opt3;

	memcpy(&data_buf[20], buf, len);		

	pthread_mutex_lock(&client_mutex);
	if(_serv  && _serv->rtsp_pa)
	{
		send(_serv->fd,buf, len,0);
	}
	pthread_mutex_unlock(&client_mutex);
}

int strcmp_describe(struct serv_struct* _serv)
{
	int ret = 1;
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		ret = strcmp(_serv->rtsp_pa->order,ORD_DESCRIBE);
	pthread_mutex_unlock(&client_mutex);

	return ret;

}

void get_client_ip(struct serv_struct* _serv,char* ip)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		strcpy(ip,_serv->client_ip);
	pthread_mutex_unlock(&client_mutex);
}
int get_session_id(struct serv_struct* _serv)
{
	int session_id = 0;

	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		session_id = _serv->session_id;
	pthread_mutex_unlock(&client_mutex);
	return session_id;
}

int get_which_track(struct serv_struct* _serv)
{
	int which_track = -1;
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
	{
		char* tmp = _serv->rtsp_pa->order_para + strlen(_serv->rtsp_pa->order_para) - 6;
		if(0 == strcmp(tmp,"track0"))
			which_track = 0;		
		else if(0 == strcmp(tmp,"track1"))
			which_track = 1;
	}
	pthread_mutex_unlock(&client_mutex);
	return which_track;
}

int strcmp_setup(struct serv_struct* _serv)
{
	int ret = 1;

	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		ret = strcmp(_serv->rtsp_pa->order,ORD_SETUP);

	pthread_mutex_unlock(&client_mutex);
	return ret;
}

void sprintf_session_id(char* ptr,struct serv_struct* _serv)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		sprintf(ptr,"Session: %d",_serv->session_id);
	pthread_mutex_unlock(&client_mutex);
}

void memcpy_transport(char* buf,struct serv_struct* _serv)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		memcpy(buf,_serv->rtsp_pa->transport,MAX_TRANSPORT_LEN);
	pthread_mutex_unlock(&client_mutex);
}

int sscanf_client_port(char* client_port,struct serv_struct* _serv,int which_track)
{
	int port = 0;

	int tmp;
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
	{
		if(0 == which_track)
		{
			sscanf(client_port,"%d-%d",&_serv->rtsp_pa->client_port0,&tmp);
			port = _serv->rtsp_pa->client_port0;
		}
		else if(1 == which_track)
		{
			sscanf(client_port,"%d-%d",&_serv->rtsp_pa->client_port1,&tmp);
			port = _serv->rtsp_pa->client_port1;
		}
	}
	pthread_mutex_unlock(&client_mutex);

	return port;
}
int sscanf_tcp_channel(char* channel,struct serv_struct* _serv)
{
	int tcp_channel = -1;
	int tmp;
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
	{
		sscanf(channel,"%d-%d",&_serv->rtsp_pa->tcp_channel,&tmp);
		tcp_channel = _serv->rtsp_pa->tcp_channel;
	}
	pthread_mutex_unlock(&client_mutex);
	return tcp_channel;
}
int strcmp_play(struct serv_struct* _serv)
{
	int ret = 1;

	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		ret = strcmp(_serv->rtsp_pa->order,ORD_PLAY);
	pthread_mutex_unlock(&client_mutex);
	return ret;
}
int strcmp_get_paramter(struct serv_struct* _serv)
{
	int ret = 1;

	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		ret = strcmp(_serv->rtsp_pa->order,ORD_GET_PARAMETER);
	pthread_mutex_unlock(&client_mutex);
	return ret;
}
int strcmp_teardown(struct serv_struct* _serv)
{
	int ret = 1;

	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		ret = strcmp(_serv->rtsp_pa->order,ORD_TEARDOWN);
	pthread_mutex_unlock(&client_mutex);
	return ret;
}
int strcmp_pause(struct serv_struct* _serv)
{
	int ret = 1;

	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		ret = strcmp(_serv->rtsp_pa->order,ORD_PAUSE);
	pthread_mutex_unlock(&client_mutex);
	return ret;
}
void set_client_stat(struct serv_struct* _serv,enum play_stat stat)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		_serv->playstat = stat;
	pthread_mutex_unlock(&client_mutex);
}
void set_client_istcp(struct serv_struct* _serv)
{
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		_serv->rtsp_pa->istcp = 1;
	pthread_mutex_unlock(&client_mutex);
}
int get_client_istcp(struct serv_struct* _serv)
{
	int ret = 0;
	pthread_mutex_lock(&client_mutex);
	if(_serv && _serv->rtsp_pa)
		ret = _serv->rtsp_pa->istcp;
	pthread_mutex_unlock(&client_mutex);
	return ret;
}
unsigned short get_h264_seq(char dt)
{
	unsigned short ret = 0;
	ret = dt == 0? rtp_pa.h264_seq_num:rtp_pa.pcm_seq_num;
	return ret;
}
void increase_h264_seq(char dt)
{
	if(dt == 0)
		rtp_pa.h264_seq_num++;
	else if(dt == 1)
		rtp_pa.pcm_seq_num++; 
}
unsigned int get_h264_timestmp(char dt)
{
	int ret =0;
	if(dt == 0)
		ret = rtp_pa.h264_timestamp;
	else
		ret = rtp_pa.pcm_timestamp;
	return ret;
}

void increase_h264_timestamp(char dt)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	double val =0;
	if(dt == 0)
	{
		val = (now.tv_sec - rtp_pa.firstTime.tv_sec) +	(now.tv_usec - rtp_pa.firstTime.tv_usec) / 1000000.0;
		rtp_pa.h264_timestamp += val * 90000.0;
		//rtp_pa.h264_timestamp += 6000;
		rtp_pa.firstTime = now;
	}
	else
	{
//		val = (now.tv_sec - rtp_pa.pcm_firstTime.tv_sec) +	(now.tv_usec - rtp_pa.pcm_firstTime.tv_usec) / 1000000.0;
		//		rtp_pa.pcm_timestamp += val * (16000.0);
		rtp_pa.pcm_timestamp += 160;
		rtp_pa.pcm_firstTime = now;
	}

}

#if 0
int rtp_send_data(char* buf,int len,char dt)
{
	int ret = 0;
	int i=0;
	struct serv_struct* _serv;
	struct timeval tv;
	char * tmp = buf-4;

	//pthread_mutex_lock(&client_mutex);

	//pthread_mutex_unlock(&client_mutex);
	return 0;
}
	for(i=0;i<MAX_RTSP_CLIENT;i++)
	{
		_serv = rtsp_clients[i];
		if(_serv &&  _serv->rtsp_pa )
		{
			if(!_serv->rtsp_pa->istcp)
			{
				if(_serv->rtsp_pa->client_port0 > 0)
				{
					if(_serv->udp_sock > 0 && _serv->playstat == PLAY )
					{
						//通过网络将数据发送出去
						if(dt == 0)
							sendto(_serv->udp_sock,buf, len, 0,(struct sockaddr *)&_serv->udp_sockaddr0,sizeof(struct sockaddr_in));//发送rtp包	
						else if(dt == 1)
							sendto(_serv->udp_sock1,buf, len, 0,(struct sockaddr *)&_serv->udp_sockaddr1,sizeof(struct sockaddr_in));//发送rtp包	
					}
				}
			}
			else
			{
#if 0
				if(_serv->bev && _serv->playstat == PLAY )
				{
					tmp[0]= 0x24;
					tmp[1] = dt == 0? 0x00 : 0x02;
					*(unsigned short*)(&tmp[2]) = htons(len);//len;

					send(_serv->fd,tmp, len+4,0);//发送rtp包
				}
#endif
			}
		}	

		pthread_mutex_unlock(&client_mutex);
		return ret;
	}
}

void check_clients_stat()
{
	int i = 0 ;
	struct serv_struct* _serv;
	int csock;

	pthread_mutex_lock(&client_mutex);

	for(i=0;i<MAX_RTSP_CLIENT;i++) 
	{
		_serv = rtsp_clients[i];	

		if(_serv && _serv->rtsp_pa)
		{
			if(_serv->rtsp_pa->istcp == 0)
			{
				if( _serv->rtsp_pa->client_port0 > 0)
				{
					if(_serv->udp_sock == -1 && _serv->playstat == PLAY )
					{
						//初次播放，初始化UDPsock
						if ( (_serv->udp_sock =socket(AF_INET, SOCK_DGRAM, 0)) <0)
						{
							pthread_mutex_unlock(&client_mutex);
							return;
						}
						memset(&_serv->udp_sockaddr0,0,sizeof(_serv->udp_sockaddr0));
						_serv->udp_sockaddr0.sin_family=AF_INET;
						_serv->udp_sockaddr0.sin_port=htons(_serv->rtsp_pa->client_port0);	
						_serv->udp_sockaddr0.sin_addr.s_addr = inet_addr(_serv->client_ip); 

						if ( (_serv->udp_sock1 =socket(AF_INET, SOCK_DGRAM, 0)) <0)	
						{
							pthread_mutex_unlock(&client_mutex);
							return;
						}
						memset(&_serv->udp_sockaddr1,0,sizeof(_serv->udp_sockaddr1));
						_serv->udp_sockaddr1.sin_family=AF_INET;
						_serv->udp_sockaddr1.sin_port=htons(_serv->rtsp_pa->client_port1);	
						_serv->udp_sockaddr1.sin_addr.s_addr = inet_addr(_serv->client_ip); 
						DEBUG("udp0 port %d,udp1 port %d \n",_serv->rtsp_pa->client_port0,_serv->rtsp_pa->client_port1);
					}
				}	
			}
		}
	}
	pthread_mutex_unlock(&client_mutex);
}

#endif


int rtp_send_data(char *buf, int len, char dt)
{
	int ret = 0;
	int i = 0;
	struct serv_struct *_serv;
	struct timeval tv;

	_serv = rtsp_client;
	if(_serv && _serv->rtsp_pa)
	{
		if(!_serv->rtsp_pa->istcp)
		{
			   if(_serv->rtsp_pa->client_port0 > 0)
                {
                    if(_serv->udp_sock > 0 && _serv->playstat == PLAY )
                    {
                        //通过网络将数据发送出去
                        if(dt == 0)
                            sendto(_serv->udp_sock,buf, len, 0,(struct sockaddr *)&_serv->udp_sockaddr0,sizeof(struct sockaddr_in));//发送rtp包 
                        else if(dt == 1)
                            sendto(_serv->udp_sock1,buf, len, 0,(struct sockaddr *)&_serv->udp_sockaddr1,sizeof(struct sockaddr_in));//发送rtp包    
                    }
                }
		}
	}
}



void check_clients_stat()
{
	struct serv_struct* _serv;
	struct sockaddr_in server;
	int csock;

	pthread_mutex_lock(&client_mutex);
	
	_serv = rtsp_client;
		
	if(_serv && _serv->rtsp_pa)
	{
		if(!_serv->rtsp_pa->istcp)
		{
			if(_serv->rtsp_pa->client_port0 > 0)
			{
			   			//初次播放，初始化UDPsock
                        if ( (_serv->udp_sock =socket(AF_INET, SOCK_DGRAM, 0)) <0)
                        {
                            pthread_mutex_unlock(&client_mutex);
                            return;
                        }
                        memset(&_serv->udp_sockaddr0,0,sizeof(_serv->udp_sockaddr0));
                        _serv->udp_sockaddr0.sin_family=AF_INET;
                        _serv->udp_sockaddr0.sin_port=htons(_serv->rtsp_pa->client_port0);  
                        _serv->udp_sockaddr0.sin_addr.s_addr = inet_addr(_serv->client_ip); 

                        if ( (_serv->udp_sock1 =socket(AF_INET, SOCK_DGRAM, 0)) <0) 
                        {
                            pthread_mutex_unlock(&client_mutex);
                            return;
                        }
                        memset(&_serv->udp_sockaddr1,0,sizeof(_serv->udp_sockaddr1));
                        _serv->udp_sockaddr1.sin_family=AF_INET;
                        _serv->udp_sockaddr1.sin_port=htons(_serv->rtsp_pa->client_port1);  
                        _serv->udp_sockaddr1.sin_addr.s_addr = inet_addr(_serv->client_ip); 
                        DEBUG("udp0 port %d,udp1 port %d \n",_serv->rtsp_pa->client_port0,_serv->rtsp_pa->client_port1);
			}
		}
	}
	pthread_mutex_unlock(&client_mutex);
}
