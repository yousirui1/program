#ifndef __H264_H__
#define __H264_H__

#include "rtsp_config.h"
#include "typedef.h"
#define PACKET_BUFFER_END            (unsigned int)0x00000000
#define MAX_RTP_PKT_LENGTH     1400

#define H264                    96
#define PCM                   8 

#define H264_SSRC 0x69257765
#define PCM_SSRC 0x69257766

#define NALU_SIZE 1024*1024
#pragma pack(1)

typedef struct 
{
	unsigned char csrc_len:4;        /**//* expect 0 */
	unsigned char extension:1;        /**//* expect 1, see RTP_OP below */
	unsigned char padding:1;        /**//* expect 0 */
	unsigned char version:2;        /**//* expect 2 */

	unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
	unsigned char marker:1;        /**//* expect 1 */

	unsigned short seq_no;

	unsigned  long timestamp;

	unsigned long ssrc;
}RTP_FIXED_HEADER;

typedef struct 
{
	unsigned char TYPE:5;
	unsigned char NRI:2;
	unsigned char F:1; 
}NALU_HEADER; /**//* 1 BYTES */

typedef struct 
{
	unsigned char TYPE:5;
	unsigned char NRI:2; 
	unsigned char F:1;    
}FU_INDICATOR; /**//* 1 BYTES */

typedef struct 
{
	unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1; 
}FU_HEADER;

typedef struct
{
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned max_size;            //! Nal Unit Buffer size
	int forbidden_bit;            //! should be always FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int nal_unit_type;            //! NALU_TYPE_xxxx    
	char *buf;                    //! contains the first byte followed by the EBSP
	//char buf[250*1024];
	unsigned short lost_packets;  //! true, if packet loss is detected
} NALU_t;

extern RTP_FIXED_HEADER        *rtp_hdr;

extern NALU_HEADER		*nalu_hdr;
extern FU_INDICATOR	*fu_ind;
extern FU_HEADER		*fu_hdr;

NALU_t *AllocNALU(int buffersize);
void h264_send_data(char* data,int len,int datatype);
#endif
