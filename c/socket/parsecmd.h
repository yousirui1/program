#ifndef __PARSE_CMD_H__
#define __PARSE_CMD_H__

#define ORD_OPTIONS "OPTIONS"
#define ORD_DESCRIBE "DESCRIBE"
#define ORD_SETUP "SETUP"
#define ORD_PLAY  "PLAY"
#define ORD_GET_PARAMETER  "GET_PARAMETER"
#define ORD_TEARDOWN "TEARDOWN"
#define ORD_PAUSE "PAUSE"

#define PARA_CSEQ "CSeq"
#define PARA_SESSION "Session"
#define PARA_TRANSPORT "Transport"

#define SERV_ERROR_RESP "400 Bad Request"

int get_free_num();
void strcpy_order(struct serv_struct* _serv,char* para);
void sscanf_cseq(char* buf,struct serv_struct* _serv);
void strcpy_session(struct serv_struct* _serv,char* para);
void strcpy_transport(struct serv_struct* _serv,char* para);
void sprintf_cseq(char* ptr,struct serv_struct* _serv);
int strcmp_options(struct serv_struct* _serv);
void client_bev_write(struct serv_struct* _serv, char* buf,int len);
int strcmp_setup(struct serv_struct* _serv);
int strcmp_describe(struct serv_struct* _serv);
void sprintf_session_id(char* ptr,struct serv_struct* _serv);
void memcpy_transport(char* buf,struct serv_struct* _serv);
int sscanf_client_port(char* client_port,struct serv_struct* _serv,int which_track);
int strcmp_play(struct serv_struct* _serv);
void set_client_stat(struct serv_struct* _serv,enum play_stat stat);
int strcmp_get_paramter(struct serv_struct* _serv);
int strcmp_teardown(struct serv_struct* _serv);
int strcmp_pause(struct serv_struct* _serv);
int sscanf_tcp_channel(char* channel,struct serv_struct* _serv);

unsigned short get_h264_seq(char dt);
unsigned int get_h264_timestmp(char dt);
void increase_h264_seq(char dt);
void increase_h264_timestamp(char dt);
int rtp_send_data(char* buf,int len,char dt);
int get_session_id(struct serv_struct* _serv);
void get_client_ip(struct serv_struct* _serv,char* ip);
int get_which_track(struct serv_struct* _serv);
void strcpy_order_para(struct serv_struct* _serv,char* para);
void set_client_istcp(struct serv_struct* _serv);
int get_client_istcp(struct serv_struct* _serv);

void pcm_send_data(char* data,int size,char dt);

int parse_cmd(struct serv_struct * _serv,char* msg,int len);
void process_cmd(struct serv_struct * _serv);
void check_clients_stat();
void clear_client_with_no_mutex(struct bufferevent *bev);

extern pthread_mutex_t client_mutex;
extern struct serv_struct* rtsp_clients[MAX_RTSP_CLIENT];
#endif

