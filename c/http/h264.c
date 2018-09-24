#include <stdlib.h>
#include "string.h"
#include "h264.h"
#include "parsecmd.h"
#include "audio.h"

static int FindStartCode2 (unsigned char *Buf);//���ҿ�ʼ�ַ�0x000001
static int FindStartCode3 (unsigned char *Buf);//���ҿ�ʼ�ַ�0x00000001

extern struct rtp_param rtp_pa;

static int info2=0, info3=0;
RTP_FIXED_HEADER        *rtp_hdr;

NALU_HEADER		*nalu_hdr;
FU_INDICATOR	*fu_ind;
FU_HEADER		*fu_hdr;

NALU_t *_n;
int lastvids_size;
void send_pcm(char* sendbuf,int len);

NALU_t *AllocNALU(int buffersize)
{
	/*NALU_t *n;
	*/
	if(!_n)
	{
		if ((_n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)
		{
			printf("AllocNALU: failed \n");
			return NULL;
		}
	}

	_n->max_size=buffersize;

	if(buffersize > lastvids_size)
	{
		if(lastvids_size >0)
		{
			free(_n->buf);
		}
		if ((_n->buf = (char*)calloc (2*buffersize+128, sizeof (char))) == NULL)
		{
			free (_n);
			_n = NULL;
			printf ("AllocNALU: n->buf -- > %d\n",2*buffersize);
			return NULL;
		}
		lastvids_size = buffersize;
	}
	//memset(_n->buf,0,lastvids_size);
	return _n;
}
void FreeNALU(NALU_t *n)
{
	if (n)
	{
		if (n->buf)
		{
			free(n->buf);

			n->buf=NULL;
		}
		free (n);
	}
}

//�����������Ϊһ��NAL�ṹ�壬��Ҫ����Ϊ�õ�һ��������NALU��������NALU_t��buf�У���ȡ���ĳ��ȣ����F,IDC,TYPEλ��
int GetAnnexbNALU (NALU_t *nalu,char* data,int len,int* readcnt)
{
	//memset(Buf,0,NALU_SIZE);
	int pos = 0;
	int StartCodeFound, rewind;
	unsigned char *Buf;

	//if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL) 
	//{
	//	printf ("GetAnnexbNALU: Could not allocate Buf memory,size = %d \n",nalu->max_size);
	//}
	Buf = nalu->buf+nalu->max_size+128/2;

	nalu->startcodeprefix_len=3;//��ʼ���������еĿ�ʼ�ַ�Ϊ3���ֽ�


	unsigned char* tmp_data = data;
	unsigned char* tmp_buf = Buf;

	int i;
	for(i=0;i<3;i++)
	{
		tmp_buf[i] = tmp_data[i];
	}
	*readcnt +=3;
	info2 = FindStartCode2 (tmp_buf);

	if(info2 != 1) 
	{
		//������ǣ��ٶ�һ���ֽ�
		*readcnt += 1;
		tmp_buf[3] = tmp_data[3];
		info3 = FindStartCode3 (tmp_buf);//�ж��Ƿ�Ϊ0x00000001
		if (info3 != 1)//������ǣ�����-1
		{ 
			//		free(Buf);
			return -1;
		}
		else 
		{
			//�����0x00000001,�õ���ʼǰ׺Ϊ4���ֽ�
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}

	else
	{
		//�����0x000001,�õ���ʼǰ׺Ϊ3���ֽ�
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}

	//������һ����ʼ�ַ��ı�־λ
	StartCodeFound = 0;
	info2 = 0;
	info3 = 0;

	while (!StartCodeFound)
	{
		//if (feof (bits))//�ж��Ƿ����ļ�β���ļ��������򷵻ط�0ֵ�����򷵻�0
		if((*readcnt) == len)
		{
			nalu->len = (pos-1)-nalu->startcodeprefix_len;  //NALU��Ԫ�ĳ��ȡ�
			memcpy (nalu->buf, &tmp_buf[nalu->startcodeprefix_len], nalu->len);     
			nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
			//		free(Buf);
			return pos-1;
		}
		tmp_buf[pos++] = data[*readcnt];
		(*readcnt)++;
		info3 = FindStartCode3(&tmp_buf[pos-4]);//�ж��Ƿ�Ϊ0x00000001
		if(info3 != 1)
		{
			info2 = FindStartCode2(&tmp_buf[pos-3]);//�ж��Ƿ�Ϊ0x000001
		}

		StartCodeFound = (info2 == 1 || info3 == 1);
	}

	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	rewind = (info3 == 1)? -4 : -3;

	(*readcnt) += rewind;

	// Here the Start code, the complete NALU, and the next start code is in the Buf.  
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code

	nalu->len = (pos+rewind)-nalu->startcodeprefix_len;    //NALU���ȣ�������ͷ����
	memcpy (nalu->buf, &tmp_buf[nalu->startcodeprefix_len], nalu->len);//����һ������NALU����������ʼǰ׺0x000001��0x00000001
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
	//	free(Buf);

	return (pos+rewind);//����������ʼ�ַ�֮�������ֽ�������������ǰ׺��NALU�ĳ���
}
static int FindStartCode2 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //�ж��Ƿ�Ϊ0x000001,����Ƿ���1
	else return 1;
}

static int FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//�ж��Ƿ�Ϊ0x00000001,����Ƿ���1
	else return 1;
}

void dump(NALU_t *n)
{
	if (!n)return;
	printf(" len: %d  ", n->len);
	printf("nal_unit_type: %x\n", n->nal_unit_type);
}
void h264_send_data(char* data,int len,int datatype)
{
	if(get_free_num() == MAX_RTSP_CLIENT)
	   return;
	
	NALU_t *n;
	char dt = datatype & 0x01 ;

	char* nalu_payload;  
	char send_buf[1500];
	char* sendbuf = &send_buf[0] + 4;//+sizeof(struct InterLeaved) ;

	int	bytes=0;

	//����ʱ���
	n = AllocNALU(len);
	if(!n)
		return;
	int readcnt = 0;
	while(readcnt != len)
	{
		GetAnnexbNALU(n,data,len,&readcnt);//ÿִ��һ�Σ��ļ���ָ��ָ�򱾴��ҵ���NALU��ĩβ����һ��λ�ü�Ϊ�¸�NALU����ʼ��0x000001
		//dump(n);//���NALU���Ⱥ�TYPE
		//��1��һ��NALU����һ��RTP��������� RTP_FIXED_HEADER��12�ֽڣ�  + NALU_HEADER��1�ֽڣ� + EBPS
		//��2��һ��NALU�ֳɶ��RTP��������� RTP_FIXED_HEADER ��12�ֽڣ� + FU_INDICATOR ��1�ֽڣ�+  FU_HEADER��1�ֽڣ� + EBPS(1400�ֽ�)

		memset(send_buf,0,1500);//���sendbuf����ʱ�Ὣ�ϴε�ʱ�����գ������Ҫts_current�������ϴε�ʱ���ֵ
		sendbuf = send_buf+4 ;

		//rtp�̶���ͷ��Ϊ12�ֽ�,�þ佫sendbuf[0]�ĵ�ַ����rtp_hdr���Ժ��rtp_hdr��д�������ֱ��д��sendbuf��
		rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 

		//����RTP HEADER��
		rtp_hdr->payload     = dt == 0 ? H264 : PCM;  //�������ͺţ�
		rtp_hdr->version     = 2;  //�汾�ţ��˰汾�̶�Ϊ2
		rtp_hdr->marker    = 0;   //��־λ���ɾ���Э��涨��ֵ��
		rtp_hdr->ssrc        = htonl(dt == 0 ? H264_SSRC : PCM_SSRC);    //���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ

		//	��һ��NALUС��1400�ֽڵ�ʱ�򣬲���һ����RTP������
		if(n->len <= 1400)
		{
			//����rtp M λ��
			rtp_hdr->marker = 1;
			rtp_hdr->seq_no  = htons(get_h264_seq(dt)); //���кţ�ÿ����һ��RTP����1��htons���������ֽ���ת�������ֽ���

			increase_h264_seq(dt);
			//����NALU HEADER,�������HEADER����sendbuf[12]
			nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����nalu_hdr��֮���nalu_hdr��д��ͽ�д��sendbuf�У�
			nalu_hdr->F = n->forbidden_bit;
			nalu_hdr->NRI=n->nal_reference_idc>>5;//��Ч������n->nal_reference_idc�ĵ�6��7λ����Ҫ����5λ���ܽ���ֵ����nalu_hdr->NRI��
			nalu_hdr->TYPE=n->nal_unit_type;

			nalu_payload=&sendbuf[13];//ͬ��sendbuf[13]����nalu_payload
			memcpy(nalu_payload,n->buf+1,n->len-1);//ȥ��naluͷ��naluʣ������д��sendbuf[13]��ʼ���ַ�����

			increase_h264_timestamp(dt);

			rtp_hdr->timestamp=htonl( get_h264_timestmp(dt));
			bytes=n->len + 12 ;	//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ�����NALUͷ����ȥ��ʼǰ׺������rtp_header�Ĺ̶�����12�ֽ�

			rtp_send_data(sendbuf,bytes,dt, 1);	
		}
		else if(n->len > 1400)  //�����Ҫ�ֳɶ��RTP�������ˡ�
		{
			//�õ���nalu��Ҫ�ö��ٳ���Ϊ1400�ֽڵ�RTP��������
			int k = 0, last = 0;
			k = n->len / 1400;//��Ҫk��1400�ֽڵ�RTP��������Ϊʲô����1�أ���Ϊ�Ǵ�0��ʼ�����ġ�
			last = n->len % 1400;//���һ��RTP������Ҫװ�ص��ֽ���
			int t = 0;//����ָʾ��ǰ���͵��ǵڼ�����ƬRTP��

			increase_h264_timestamp(dt);
			rtp_hdr->timestamp=htonl(get_h264_timestmp(dt));

			while(t <= k)
			{
				rtp_hdr->seq_no = htons(get_h264_seq(dt)); //���кţ�ÿ����һ��RTP����1
				increase_h264_seq(dt);
				if(!t)//����һ����Ҫ��Ƭ��NALU�ĵ�һ����Ƭ����FU HEADER��Sλ,t = 0ʱ������߼���
				{
					//����rtp M λ��
					rtp_hdr->marker = 0;  //���һ��NALUʱ����ֵ���ó�1�����������ó�0��
					//rtp_hdr->marker = 1;

					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F = n->forbidden_bit;
					fu_ind->NRI = n->nal_reference_idc >> 5;
					fu_ind->TYPE = 28;  //FU-A���͡�

					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->E = 0;
					fu_hdr->R = 0;
					fu_hdr->S = 1;
					fu_hdr->TYPE = n->nal_unit_type;

					nalu_payload = &sendbuf[14];//ͬ��sendbuf[14]����nalu_payload
					memcpy(nalu_payload,n->buf+1,1400);//ȥ��NALUͷ��ÿ�ο���1400���ֽڡ�

					bytes = 1400 + 14;//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥ��ʼǰ׺��NALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����                                                            14�ֽ�

					rtp_send_data(sendbuf,bytes,dt, 1);	
					t++;
				}
				//����һ����Ҫ��Ƭ��NALU�ķǵ�һ����Ƭ������FU HEADER��Sλ������÷�Ƭ�Ǹ�NALU�����һ����Ƭ����FU HEADER��Eλ
				else if(k == t)//���͵������һ����Ƭ��ע�����һ����Ƭ�ĳ��ȿ��ܳ���1400�ֽڣ��� l> 1386ʱ����
				{
					//����rtp M λ����ǰ����������һ����Ƭʱ��λ��1
					rtp_hdr->marker=1;
					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;
					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr = (FU_HEADER*)&sendbuf[13];
					fu_hdr->R = 0;
					fu_hdr->S = 0;
					fu_hdr->TYPE = n->nal_unit_type;
					fu_hdr->E = 1;

					nalu_payload = &sendbuf[14];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
					memcpy(nalu_payload,n->buf + t*1400 + 1,last-1);//��nalu���ʣ���l-1(ȥ����һ���ֽڵ�NALUͷ)�ֽ�����д��sendbuf[14]��ʼ���ַ�����
					bytes = last - 1 + 14;		//���sendbuf�ĳ���,Ϊʣ��nalu�ĳ���l-1����rtp_header��FU_INDICATOR,FU_HEADER������ͷ��14�ֽ�

					rtp_send_data(sendbuf,bytes,dt, 1);	
					t++;
				}
				//�Ȳ��ǵ�һ����Ƭ��Ҳ�������һ����Ƭ�Ĵ���
				else if(t < k && 0 != t)
				{
					//����rtp M λ��
					rtp_hdr->marker = 0;
					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind = (FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F = n->forbidden_bit;
					fu_ind->NRI = n->nal_reference_idc>>5;
					fu_ind->TYPE = 28;


					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];

					fu_hdr->R = 0;
					fu_hdr->S = 0;
					fu_hdr->E = 0;
					fu_hdr->TYPE = n->nal_unit_type;

					nalu_payload=&sendbuf[14];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
					memcpy(nalu_payload, n->buf + t * 1400 + 1,1400);//ȥ����ʼǰ׺��naluʣ������д��sendbuf[14]��ʼ���ַ�����
					bytes=1400 + 14;	//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥԭNALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�

					rtp_send_data(sendbuf,bytes,dt, 1);	
					t++;
				}
			}
		}
	}
	//	FreeNALU(n);
}

AUDIO_BUF stAudioBuf;

void send_pcm_data(char *data, int len, char dt, int channel)
{
	if(get_free_num() == MAX_RTSP_CLIENT)
		return;

	char data_buf[4000] = {0};	

	char *databuf = &data_buf[0];

	char temp[500] = {0};

	int i, size = 0;

	for(i = 0; i < AUDIO_MAX_BUFCNT; i++)
	{
		temp[0] = 0x24;
		temp[1] = 0;
		temp[2] = 0;
		temp[3] = 0;
		
		*(unsigned int*)(&temp[4]) =htonl(170 +2);
		temp[8] = 0x80;
        temp[9] = 0x08;

		RTP_HTTP_HEADER *rtp_hdr;
    	rtp_hdr =(RTP_HTTP_HEADER*)&temp[10];
		increase_h264_seq(1);
    	rtp_hdr->seq_no = htons(get_h264_seq(1));
		increase_h264_timestamp(1);       
    	rtp_hdr->timestamp=htonl(get_h264_timestmp(1)); 
    	rtp_hdr->ssrc= htonl(HTTP_PCM_SSRC);//���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ   
		memcpy(&temp[20], data, 160);
		data += 160;
		memcpy(databuf, temp, 180);	
		databuf += 180;	
	}
	
	len += 20 * AUDIO_MAX_BUFCNT;	

	char *test = &data_buf[0];


	while(len)
	{
		if(len > 1400)
		{
			rtp_send_data(test, 1400, dt, channel);	
			test +=1400;	
			len -= 1400;

		}
		else if(len <= 1400)
		{
			rtp_send_data(test, len, dt, channel);	
			len -= len;
		}

	}	

}


void send_data(char*data, int len,char dt, int channel)
{
	if(get_free_num() == MAX_RTSP_CLIENT)
	   return;
	char send_buf[1500] = {0};
	char* sendbuf = &send_buf[0]+ 10; 

	RTP_HTTP_HEADER *rtp_hdr;

	rtp_hdr =(RTP_HTTP_HEADER*)&sendbuf[0];

	if(channel == 1)
	{
		increase_h264_timestamp(dt);
		increase_h264_seq(dt);
	}
	
	rtp_hdr->seq_no = htons(get_h264_seq(dt));

	rtp_hdr->timestamp=htonl(get_h264_timestmp(dt)); 

	if(dt == 0)
	{
		rtp_hdr->ssrc= htonl(HTTP_H264_SSRC);//���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ   
	}
	else 
	{
		rtp_hdr->ssrc= htonl(HTTP_PCM_SSRC);//���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ   

	}
	
	while(len)
	{

		if(len > 1400)
		{
			memcpy(&sendbuf[10], data, 1400);	
			rtp_send_data(sendbuf, 1410, dt, channel);
			len -= 1400;
			data += 1400;
		}
		else if(len <= 1400)
		{
			memcpy(&sendbuf[10], data, len);	
			rtp_send_data(sendbuf, len + 10, dt,channel);
			len -= len;
		}
	}
}
void pcm_send_data(char* data,int size,char dt)
{
	if(get_free_num() == MAX_RTSP_CLIENT)
	   return;
	char send_buf[500] = {0};
	char* sendbuf = &send_buf[0]+4; 

	memcpy(&sendbuf[12],data,size);	
	send_pcm(sendbuf,size);

}
void send_pcm(char* sendbuf,int len )
{
	int M_bit = 1; 
	RTP_FIXED_HEADER*rtp_hdr;

	M_bit = 1;

	rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 

	rtp_hdr->payload = PCM;//�������ͺţ�   
	rtp_hdr->version = 2;//�汾�ţ��˰汾�̶�Ϊ2   
	rtp_hdr->marker= 1; //��־λ���ɾ���Э��涨��ֵ��   

	rtp_hdr->ssrc= htonl(PCM_SSRC);//���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ   

	increase_h264_seq(1);
	rtp_hdr->seq_no = htons(get_h264_seq(1));

	rtp_hdr->timestamp=htonl(get_h264_timestmp(1)); 
	increase_h264_timestamp(1);

	rtp_send_data(sendbuf,len+12,1 , 1);
}

unsigned long GetTickCount()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

