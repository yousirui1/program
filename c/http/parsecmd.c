#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "rtsp_config.h"
#include "parsecmd.h"
#include "typedef.h"
#include "h264.h"
#include <netinet/tcp.h>

void SendErrmsg(struct serv_struct * _serv);
void sprintf_date(char* ptr);

extern struct rtp_param rtp_pa;

#define SDP_EL "\r\n"

struct bufferevent * bev_channel_1;
struct bufferevent * bev_channel_2;

int parse_http(struct bufferevent *bev, char *msg, int len)
{
	if(!bev)
		return 1;
	
	char delims[] = "\r\n";
		
	char *rows = NULL;
	
	rows = strtok(msg, delims);

	if(strstr(rows, "getidentify.cgi"))
	{
		process_http(bev, 1, 0);	
	}
		
	else if(strstr(rows, "param.cgi"))
	{
		if(strstr(rows, "getimageattr"))
		{
			
			process_http(bev, 2, 1);	
		}	
		else if(strstr(rows, "getoverlayattr"))
		{
		
			process_http(bev, 2, 2);	
		}	
		else if(strstr(rows, "setservertime"))
		{
			rows =  strstr(rows, "&-time");
			char servertime[20] = {0};
			sscanf(rows, "&-time=%s", servertime);

			set_system_time(servertime);
			process_http(bev, 2, 3);	
		}
		
	}

	else if(strstr(rows, "livestream"))
	{

		if(strstr(rows, "11"))
		{
			process_http(bev, 3, 1);	
		}
		else if(strstr(rows, "12"))
		{
			process_http(bev, 3, 2);	
		}
	}

	return 0;

}

void process_http(struct bufferevent *bev,  int cmd, int type)
{

	int fd = bufferevent_getfd(bev);

	char buf[1024] = {0};
	char *ptr = buf;
	int sl = 0;

	char value[1024] = {0};

	if(cmd != livestream)
	{
		
		if(cmd == getidentify)
		{
			char productid[20] = {0};
			read_profile_string(CGI_SECTION_REC, "dev_id", productid, sizeof(productid), "", REC_CONFIG_FILE);	
			sprintf(value, "var productid=\"%s\";"SDP_EL, productid);
			//strcpy(value, "var productid=\"C6F0SfZ3N0P5L2\";"SDP_EL);
			strcat(value, "var vendorid=\"GK\";"SDP_EL);
		}
		else if(cmd == param)
		{
			if(type == 3)
			{
				strcpy(value, "[Succeed]set ok.");
			}
			else if(type == 1)
			{
				strcpy(value, "var display_mode=\"1\";"SDP_EL);
				strcat(value, "var brightness=\"50\";"SDP_EL);
				strcat(value, "var saturation=\"1280\";"SDP_EL);
				strcat(value, "var sharpness=\"55\";"SDP_EL);
				strcat(value, "var contrast=\"50\";"SDP_EL);
				strcat(value, "var hue=\"50\";"SDP_EL);
				strcat(value, "var wdr=\"off\";"SDP_EL);
				strcat(value, "var wdrvalue=\"15\";"SDP_EL);
				strcat(value, "var night=\"off\";"SDP_EL);
				strcat(value, "var shutter=\"10000\";"SDP_EL);
				strcat(value, "var flash_shutter=\"14\";"SDP_EL);
				strcat(value, "var flip=\"on\";"SDP_EL);
				strcat(value, "var mirror=\"on\";"SDP_EL);
				strcat(value, "var gc=\"30\";"SDP_EL);
				strcat(value, "var ae=\"2\";"SDP_EL);
				strcat(value, "var targety=\"64\";"SDP_EL);
				strcat(value, "var noise=\"0\";"SDP_EL);
				strcat(value, "var gamma=\"4\";"SDP_EL);
				strcat(value, "var aemode=\"0\";"SDP_EL);
				strcat(value, "var imgmode=\"1\";"SDP_EL);
		
			}
			else if(type == 2)
			{
				strcpy(value, "var show_0=\"1\";"SDP_EL);
				strcat(value, "var place_0=\"1\";"SDP_EL);
				strcat(value, "var format_0=\"0\";"SDP_EL);
				strcat(value, "var x_0=\"976\";"SDP_EL);
				strcat(value, "var y_0=\"0\";"SDP_EL);
				strcat(value, "var name_0=\"YYYY-MM-DD hh:mm:ss\";"SDP_EL);
				strcat(value, "var show_1=\"1\";"SDP_EL);
				strcat(value, "var place_1=\"0\";"SDP_EL);
				strcat(value, "var format_1=\"0\";"SDP_EL);
				strcat(value, "var x_1=\"0\";"SDP_EL);
				strcat(value, "var y_1=\"0\";"SDP_EL);
				strcat(value, "var name_1=\"IP Camera\";"SDP_EL);
			}
			
		}


		sprintf(ptr, "%s", "HTTP/1.0 200 OK");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Server: GK");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Cache-Control: no-cache");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Content-Type:text/html");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Connection: close");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s %d", "Content-Length:", strlen(value));
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;
		ptr[0] = '\r';
		ptr[1] = '\n';
		ptr += 2;

		sprintf(ptr, "%s", value);
		sl = strlen(ptr);
		ptr += sl;
		
	

		bufferevent_write(bev, buf, ptr - buf);
		//bufferevent_free(bev);
	}
	else
	{

		if(type == 1)
		{
			strcpy(value, "Cseq: 1\r\n");
        	strcat(value, "m=video 96 H264/90000/1280/720\r\n");
        	strcat(value, "m=audio 8 G711a/8000/1\r\n");
        	strcat(value, "Transport: RTP/AVP/TCP;unicast;hisiinterleaved=0-1;ssrc=1804289383\r\n\r\n");
			get_client(bev)->channel = 1;
		}
		else if(type == 2)
		{
			strcpy(value, "Cseq: 1\r\n");
        	strcat(value, "m=video 96 H264/90000/640/352\r\n");
        	strcat(value, "m=audio 8 G711a/8000/1\r\n");
        	strcat(value, "Transport: RTP/AVP/TCP;unicast;hisiinterleaved=0-1;ssrc=1804289383\r\n\r\n");
			get_client(bev)->channel = 2;
		}	

		
		sprintf(ptr, "%s", "HTTP/1.0 200 OK");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Host: %s" , get_client(bev)->client_ip);
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Connection: Keep-Alive");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;


		sprintf(ptr, "%s", "Server: GK");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Cache-Control: no-cache");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;
	
		sprintf(ptr, "%s", "Accept-Ranges: Bytes");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;


		sprintf(ptr, "%s", "Content-Type: application/octet-stream");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Connection: close");
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;

		sprintf(ptr, "%s", "Session: %d", get_client(bev)->session_id);
		sl = strlen(ptr);
		ptr[sl] = '\r';
		ptr[sl + 1] = '\n';
		ptr += sl +2;
		ptr[0] = '\r';
		ptr[1] = '\n';
		ptr += 2;


		sprintf(ptr, "%s", value);
		sl = strlen(ptr);
		ptr += sl;

		bufferevent_write(bev, buf, ptr - buf);
		set_client_stat(get_client(bev),PLAY);
	}
}




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
		clear_client(_serv->bev);
		//clear_client_with_no_mutex(_serv->bev);
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

int rtp_send_data(char* buf,int len,char dt, int channel)
{
	int ret = 0;
	int i=0;
	struct serv_struct* _serv;
	struct timeval tv;
	
	

	pthread_mutex_lock(&client_mutex);
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
#ifdef HTTP
				char * tmp = buf- 10;
				if(_serv->bev && _serv->playstat == PLAY)
				{
					if(dt == 1)
					{
						if(_serv->channel == channel)
						{
							if(send(_serv->fd, buf, len,0) <0)
							{
								DEBUG("send resp package err !! : fd :%d \n",_serv->fd);
								_serv->playstat == STOP;
							}
						
						}

					}
					else{

					tmp[0]= 0x24;
					tmp[1] =0 ;
					tmp[2] = 0;
					tmp[3] = 0;

					*(unsigned int*)(&tmp[4]) =htonl( len +2);
					if(dt == 0)
					{
						tmp[8] = 0x80;
						tmp[9] = 0xe0;
					}
					else
					{
						tmp[8] = 0x80;
						tmp[9] = 0x08;
					}

					if(_serv->channel == channel)
					{
						if(send(_serv->fd, tmp, len + 10,0) <0)
						{
							DEBUG("send resp package err !! : fd :%d \n",_serv->fd);
							_serv->playstat == STOP;
						}
						
					}
					}
				}
#else

				char * tmp = buf-4;
				if(_serv->bev && _serv->playstat == PLAY)
				{
					tmp[0]= 0x24;
					tmp[1] = dt == 0? 0x00 : 0x02;
					*(unsigned short*)(&tmp[2]) = htons(len);
					if(send(_serv->fd, tmp, len+4,0) <0)
					{
						DEBUG("send resp package err !! : fd :%d \n",_serv->fd);
					}
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
	struct sockaddr_in server;
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


